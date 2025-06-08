#include "../include/serveract.hpp"
#include "../include/action.hpp"
#include <utility>
#include <any>
#include <algorithm>
#include <fstream>

/*
 * reactor中event的flags标记：
 * 0: 一级监听事件
 * 1: 命令控制事件，可以进行二级创建，或者读写并执行命令
 * 2: 二级监听，没什么用
 * 3: 文件传输读/写事件
 * 1 -> 2-3
 * 2 -> 1-3
*/

namespace file {
    std::string root_dir;
}

void init_root_dir(const char* str) {
    std::ifstream fs;
    fs.open(str);
    if (!fs.is_open()) {
        std::cerr << "Failed to open root directory file" << std::endl;
        throw std::runtime_error("Failed to open root directory file");
    }
    std::string dir, all;
    all.assign((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
    if (all.empty()) {
        std::cerr << "Root directory is empty" << std::endl;
        throw std::runtime_error("Root directory is empty");
    }
    auto pos1 = all.find("ServerRootDirectory=");
    if (pos1 == std::string::npos)  {
        std::cerr << "Invalid root directory format" << std::endl;
        throw std::runtime_error("Invalid root directory format");
    }
    auto pos2 = all.find('\n', pos1);
    if (pos2 != std::string::npos) {
        dir = all.substr(pos1 + 20, pos2);
    }
    else {
        dir = all.substr(pos1 + 20);
    }
    while (dir.back() == ' ') {
        dir.pop_back();
    }
    if (dir.back() != '/') {
        dir += '/';
    }
    file::root_dir = dir;
    std::cout << "Root directory initialized: " << file::root_dir << std::endl;
    fs.close();
}

void root_connection(event* ev) {
    struct sockaddr_in client_addr = {0};
    socklen_t addr_len = sizeof(client_addr);
    int cfd = accept(ev->fd, (struct sockaddr*)&client_addr, &addr_len);
    event* pe;
    do {
        if (cfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return; // No more connections to accept
            }
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            break;
        }
        int flag = fcntl(cfd, F_SETFL, O_NONBLOCK);
        if (flag < 0) {
            std::cerr << "Failed to set non-blocking mode: " << strerror(errno) << std::endl;
            break;
        }
        pe = new event(cfd, EPOLLIN | EPOLLET | EPOLLONESHOT, ev->p_rea->event_buf_size);
        if (!pe) {
            std::cerr << "Failed to create event for new connection" << std::endl;
            break;
        }
        pe->flags = 1;
        pe->set(std::bind(command_analyser, pe));
        pe->data = new std::pair<event*, event*>(nullptr, nullptr);
        pe->cntler = new file_manager(file::root_dir);
        if (!ev->p_rea->add_event(pe)) {
            std::cerr << "Failed to add event to reactor" << std::endl;
            delete pe; // Clean up if adding fails
            break;
        }
        // print message
        std::cout << "Client connected: " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;
        return;
    } while (0);
    close(cfd);
    throw std::runtime_error("Failed to accept connection - " + std::string(strerror(errno)));
}

void command_analyser(event* ev) {
    if (!ev || !ev->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return;
    }
    // 如果这里循环读取更加严格
    std::cout << "Command analyser started for event: " << ev->fd << std::endl;
    std::string command;
    ev->recv_message(command);
    std::cout << "Received command: " << command << std::endl;
    // 执行各种本地命令,设置next_event的值
    if (command == "PASV") { // 创建数据传输通道
        new_data_channel_listen(ev);
    }
    else if (command.substr(0, 4) == "STOR") { // 服务端下载
        download(ev, command.substr(5));
    }
    else if (command.substr(0, 4) == "RETR") { // 服务端上传
        upload(ev, command.substr(5));
    }
    else if (command == "ls") { // 列出当前目录下的文件
        list_dir(ev);
    }
    else if (command.substr(0, 2) == "cd") { // 切换目录
        change_dir(ev, command.substr(3));
    }
    else if (command.substr(0, 5) == "mkdir") { // 创建目录
        create_dir(ev, command.substr(6));
    }
    else if (command.substr(0, 5) == "rmdir") { // 删除目录
        remove_dir(ev, command.substr(6));
    }
    else if (command.substr(0, 2) == "rm") { // 删除文件
        remove_file(ev, command.substr(3));
    }
    else if (command == "close") { // 关闭数据通道
        close_channel(ev, true);
    }
    else if (command == "dic") { // 断开连接
        disconnect(ev);
    }
    else if (command.empty()) {
        std::cout << "Got Empty Command" << std::endl;
    }
    else {
        std::cerr << "Unknown command: " << command << std::endl;
        return;
    }
}

void new_data_channel_listen(event* ev) {
    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = 0; // 让系统自动分配端口
    serv_addr.sin_addr.s_addr = INADDR_ANY; // 绑定到所有可用地址
    int llfd = socket(AF_INET, SOCK_STREAM, 0);

    event* ll_event = nullptr;
    if (llfd < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return;
    }
    do {
        if (::bind(llfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
            break;
        }
        if (listen(llfd, 1) < 0) {
            std::cerr << "Failed to listen on socket: " << strerror(errno) << std::endl;
            break;
        }
        ll_event = new event(llfd, EPOLLIN | EPOLLET | EPOLLONESHOT, ev->p_rea->event_buf_size);
        if (ll_event == nullptr) {
            std::cerr << "Failed to create event for listening socket" << std::endl;
            break;
        }
        ll_event->flags = 2; // 二级监听事件
        ll_event->data = new std::pair<event*, event*>(ev, nullptr);
        auto ll_func = std::bind(create_data_channel, ll_event);
        ll_event->set(EPOLLIN | EPOLLET | EPOLLONESHOT, ll_func);
        if (!ev->p_rea->add_event(ll_event)) {
            std::cerr << "Failed to add event to reactor" << std::endl;
            break;
        }
        ev->remove_from_tree();
        auto func = std::bind(notify_client, ev);
        std::any_cast<std::pair<event*, event*>*>(ev->data)->first = ll_event;
        ev->set(EPOLLOUT | EPOLLET | EPOLLONESHOT, func);
        ev->add_to_tree();
        return;
    } while (0);
    close(llfd);
    if (ll_event) {
        delete ll_event;
        ll_event = nullptr;
    }
}

void notify_client(event* ev) {
    if (!ev || !ev->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return;
    }
    event* s_ev = std::any_cast<std::pair<event*, event*>*>(ev->data)->first;
    sockaddr_in serv_addr = {0};
    socklen_t addr_len = sizeof(serv_addr);
    if (getsockname(s_ev->fd, (struct sockaddr*)&serv_addr, &addr_len) < 0) {
        std::cerr << "Failed to get socket name: " << strerror(errno) << std::endl;
        return;
    }
    uint16_t port = ntohs(serv_addr.sin_port);
    std::string ip = inet_ntoa(serv_addr.sin_addr);
    std::cout << "Data channel created at " << ip << ":" << port << std::endl;
    // 通知客户端数据通道已创建
    std::string resp = "227 Entering Passive Mode ("
                       + ip + "," + std::to_string(port / 256)
                       + "," + std::to_string(port % 256) + ")";
    std::replace(resp.begin(), resp.end(), '.', ','); // 替换IP地址中的点为逗号
    send_resp(ev, resp);
    ev->remove_from_tree();
    ev->set(EPOLLIN | EPOLLET | EPOLLONESHOT, std::bind(command_analyser, ev));
    ev->add_to_tree();
    std::cout << "Client notified about data channel creation" << std::endl;
}

void create_data_channel(event* ev) {
    int new_cfd = -1;
    struct sockaddr_in clnt_addr = {0};
    event* nc_event = nullptr;
    do {
        socklen_t addr_len = sizeof(clnt_addr);
        new_cfd = accept(ev->fd, (struct sockaddr*)&clnt_addr, &addr_len);
        if (new_cfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "No connections to accept" << std::endl;
                break;
            }
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            break;
        }
        int flag = fcntl(new_cfd, F_SETFL, O_NONBLOCK);
        if (flag < 0) {
            std::cerr << "Failed to set non-blocking mode: " << strerror(errno) << std::endl;
            break;
        }
        nc_event = new event(new_cfd, 0, ev->p_rea->event_buf_size);
        if (!nc_event) {
            std::cerr << "Failed to create event for new connection" << std::endl;
            break;
        }
        nc_event->flags = 3; // 文件传输读/写事件
        // nc_event->set(EPOLLIN | EPOLLET | EPOLLONESHOT);
        nc_event->data = std::string();
        // if (ev->p_rea->add_event(nc_event) == false) {
        //     break;
        // }
        // // 暂且没有业务
        // nc_event->remove_from_tree();
        // 不加入reactor后，这个event完全是独立的，只由flag=1的event管理
        // 当封装版的socket用了
        std::any_cast<std::pair<event*, event*>*>(ev->data)->second = nc_event;
        event* p_event = std::any_cast<std::pair<event*, event*>*>(ev->data)->first;
        std::any_cast<std::pair<event*, event*>*>(p_event->data)->second = nc_event;
        // 不再次设置回调函数
        return;
    } while (0);
    if (new_cfd > 0) {
        close(new_cfd);
    }
    if (nc_event) {
        delete nc_event;
        nc_event = nullptr;
    }
}

void download(event* ev, std::string arg) {
    if (!ev || !ev->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return;
    }
    auto data = std::any_cast<std::pair<event*, event*>*>(ev->data);
    event* nc_event = data->second;
    auto fm = std::any_cast<file_manager*>(ev->cntler);
    if (!fm) {
        std::cerr << "File manager is not initialized" << std::endl;
        return;
    }
    std::string resp;
    bool exists = fm->file_exists(arg);
    if (!exists) {
        resp = "Ready to download file";
        // nc_event->set(EPOLLIN | EPOLLET | EPOLLONESHOT,
        //     std::bind(do_download, nc_event, arg, fm));
        // nc_event->add_to_tree();
        ev->p_rea->pool->submit( 
            [&] {
                fm->download(arg, nc_event->fd, nc_event->buf,
                nc_event->buffer_size, &nc_event->buflen,
                std::any_cast<std::string&>(nc_event->data), 's');
            }
        );
    }
    else {
        resp = "File already exists";
    }
    ev->remove_from_tree();
    ev->set(EPOLLOUT | EPOLLET | EPOLLONESHOT, std::bind(double_send_resp, ev, resp));
    ev->add_to_tree();
}

void upload(event* ev, std::string arg) {
    if (!ev || !ev->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return;
    }
    auto data = std::any_cast<std::pair<event*, event*>*>(ev->data);
    event* nc_event = data->second;
    auto fm = std::any_cast<file_manager*>(ev->cntler);
    if (!fm) {
        std::cerr << "File manager is not initialized" << std::endl;
        return;
    }
    std::string resp;
    bool exists = fm->file_exists(arg);
    if (exists) {
        resp = "Ready to upload file";
        // nc_event->set(EPOLLOUT | EPOLLET | EPOLLONESHOT,
        //     std::bind(do_upload, nc_event, arg, fm));
        // nc_event->add_to_tree();
        ev->p_rea->pool->submit(
            [&] {
                fm->upload(arg, nc_event->fd, nc_event->buf,
                nc_event->buffer_size, &nc_event->buflen,
                std::any_cast<std::string&>(nc_event->data), 's');
            }
        );
    }
    else {
        resp = "File does not exist";
    }
    ev->remove_from_tree();
    ev->set(EPOLLOUT | EPOLLET | EPOLLONESHOT, std::bind(double_send_resp, ev, resp));
    ev->add_to_tree();
}

void list_dir(event* ev) {
    std::string resp;
    auto fm = std::any_cast<file_manager*>(ev->cntler);
    if (!fm) {
        std::cerr << "File manager is not initialized" << std::endl;
        return;
    }
    fm->ls(resp);
    ev->remove_from_tree();
    ev->set(EPOLLOUT | EPOLLET | EPOLLONESHOT, std::bind(send_resp, ev, resp));
    ev->add_to_tree();
}

void change_dir(event* ev, std::string arg) {
    std::string resp;
    auto fm = std::any_cast<file_manager*>(ev->cntler);
    if (!fm) {
        std::cerr << "File manager is not initialized" << std::endl;
        return;
    }
    fm->cd(arg, resp);
    ev->remove_from_tree();
    ev->set(EPOLLOUT | EPOLLET | EPOLLONESHOT, std::bind(send_resp, ev, resp));
    ev->add_to_tree();
}

void create_dir(event* ev, std::string arg) {
    std::string resp;
    auto fm = std::any_cast<file_manager*>(ev->cntler);
    if (!fm) {
        std::cerr << "File manager is not initialized" << std::endl;
        return;
    }
    fm->mkdir(arg, resp);
    ev->remove_from_tree();
    ev->set(EPOLLOUT | EPOLLET | EPOLLONESHOT, std::bind(send_resp, ev, resp));
    ev->add_to_tree();
}

void remove_dir(event* ev, std::string arg) {
    std::string resp;
    auto fm = std::any_cast<file_manager*>(ev->cntler);
    if (!fm) {
        std::cerr << "File manager is not initialized" << std::endl;
        return;
    }
    fm->rmdir(arg, resp);
    ev->remove_from_tree();
    ev->set(EPOLLOUT | EPOLLET | EPOLLONESHOT, std::bind(send_resp, ev, resp));
    ev->add_to_tree();
}

void remove_file(event* ev, std::string arg) {
    std::string resp;
    auto fm = std::any_cast<file_manager*>(ev->cntler);
    if (!fm) {
        std::cerr << "File manager is not initialized" << std::endl;
        return;
    }
    fm->rm(arg, resp);
    ev->remove_from_tree();
    ev->set(EPOLLOUT | EPOLLET | EPOLLONESHOT, std::bind(send_resp, ev, resp));
    ev->add_to_tree();
}

void close_channel(event* ev, bool flag) {
    if (!ev || !ev->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return;
    }
    // 关闭数据通道
    auto pp = std::any_cast<std::pair<event*, event*>*>(ev->data);
    event* ll_event = pp->first;
    event* data_event = pp->second;
    pp->first = pp->second = nullptr;
    pp = std::any_cast<std::pair<event*, event*>*>(ll_event->data);
    pp->first = pp->second = nullptr;
    ll_event->data.reset();
    if (!ev->p_rea->remove_event(ll_event)) {
        std::cerr << "Failed to remove listening event from reactor" << std::endl;
    }
    if (!ev->p_rea->remove_event(data_event)) {
        std::cerr << "Failed to remove data event from reactor" << std::endl;
    }
    delete ll_event;
    delete data_event;
    ll_event = data_event = nullptr;
    if (flag) {
        ev->remove_from_tree();
        ev->set(EPOLLIN | EPOLLET | EPOLLONESHOT, std::bind(command_analyser, ev));
        ev->add_to_tree();
    }
    std::cout << "Data channel closed" << std::endl;
}

void disconnect(event* ev) {
    if (!ev || !ev->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return;
    }
    event* ll_event = std::any_cast<std::pair<event*, event*>*>(ev->data)->first;
    if (ll_event) {
        close_channel(ev, false); // 先关闭数据通道
        ll_event = nullptr;
    }
    if (!ev->p_rea->remove_event(ev)) {
        std::cerr << "Failed to remove event from reactor" << std::endl;
    }
    ev->data.reset();
    ev->cntler.reset();
    delete ev; // 删除事件
    ev = nullptr;
    std::cout << "Client disconnected" << std::endl;
}

void send_resp(event* ev, const std::string& resp) {
    if (!ev || !ev->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return;
    }
    ev->send_message(resp);
    ev->remove_from_tree();
    ev->set(EPOLLIN | EPOLLET | EPOLLONESHOT, std::bind(command_analyser, ev));
    ev->add_to_tree();
    std::cout << "Response sent to client: " << resp << std::endl;
}

void double_send_resp(event* ev, const std::string& resp1) {
    if (!ev || !ev->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return;
    }
    ev->send_message(resp1);
    ev->remove_from_tree();
    event* nc_event = std::any_cast<std::pair<event*, event*>*>(ev->data)->second;
    std::string& resp2 = std::any_cast<std::string&>(nc_event->data);
    while (resp2.empty() || resp2.back() != '\n') {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    ev->set(EPOLLOUT | EPOLLET | EPOLLONESHOT,
        std::bind(send_resp, ev, resp2));
    ev->add_to_tree();
    std::cout << "Former response sent to client: " << resp1 << std::endl;
}

// void do_download(event* ev, const std::string& arg, file_manager* fm) {
//     //throw std::runtime_error("do_download is not implemented yet");
//     fm->download(arg, ev->fd, ev->buf, ev->buffer_size,
//                  &ev->buflen, std::any_cast<std::string&>(ev->data), 's');
//     // 不使用, 摘下即可
//     ev->remove_from_tree();
// }

// void do_upload(event* ev, const std::string& arg, file_manager* fm) {
//     //throw std::runtime_error("do_upload is not implemented yet");
//     fm->upload(arg, ev->fd, ev->buf, ev->buffer_size,
//                &ev->buflen, std::any_cast<std::string&>(ev->data), 's');
//     // 不使用, 摘下即可
//     ev->remove_from_tree();
// }

