#include "../include/serveract.hpp"
#include "../include/action.hpp"

/*
 * reactor中event的flags标记：
 * 0: 一级监听事件
 * 1: 命令控制事件，可以进行二级创建，或者读写并执行命令
 * 2: 二级监听，没什么用
 * 3: 文件传输读/写事件
*/

// for root listener event, flags = 0
void root_connection(event* ev) {
    struct sockaddr_in client_addr = {0};
    int i;
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
        pe = new event(cfd, EPOLLIN | EPOLLET, ev->p_rea->event_buf_size);
        if (!pe) {
            std::cerr << "Failed to create event for new connection" << std::endl;
            break;
        }
        if (pe->p_rea->add_event(pe) == false) {
            delete pe;
            std::cerr << "Too many events." << std::endl;
            break;
        }
        pe->flags = 1;
        pe->set(std::bind(command_analyser, pe));
        pe->apply_to_reactor(ev->p_rea);
        // print message
        std::cout << "Client connected: " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;
        return;
    } while (0);
    close(cfd);
    throw std::runtime_error("Failed to accept connection - " + std::string(strerror(errno)));
}

/* for command analyser event, flags = 1
 * commands:
 * 1. PASV: 创建数据传输通道
 * 2. STOR <file>: 客户端上传文件(服务端下载)
 * 3. RETR <file>: 客户端下载文件(服务端上传)
 * 4. ls: 列出当前目录下的文件
 * 5. cd <dir>: 切换目录
 * 6. mkdir <dir>: 创建目录
 * 7. rmdir <dir>: 删除目录
 * 8. rm <file>: 删除文件
 * 9. dic: 断开通道
*/
void command_analyser(event* ev) {
    if (!ev || !ev->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return;
    }
    // 如果这里循环读取更加严格
    int datasize = 0;
    if (read_size_from(ev, &datasize) <= 0) {
        std::cerr << "Failed to read data size from event" << std::endl;
        return;
    }
    if (datasize <= 0 || datasize > ev->buffer_size) {
        std::cerr << "Invalid data size: " << datasize << std::endl;
        return;
    }
    if (read_from(ev) <= 0) {
        std::cerr << "Failed to read data from event" << std::endl;
        return;
    }
    std::string command = ev->buf;
    std::cout << "Received command: " << command << std::endl;
    // 执行各种本地命令,设置next_event的值
    if (command == "PASV") { // 创建数据传输通道
        //next_event = EPOLLOUT | EPOLLET;
    }
    else if (command.substr(0, 4) == "STOR") { // 服务端下载
        //next_event = EPOLLIN | EPOLLET;
    }
    else if (command.substr(0, 4) == "RETR") { // 服务端上传
        //next_event = EPOLLOUT | EPOLLET;
    }
    else if (command == "ls") { // 列出当前目录下的文件
        //next_event = EPOLLOUT | EPOLLET;
        //ev->call_back_func = std::bind(list_files, ev);
    }
    else if (command.substr(0, 2) == "cd") { // 切换目录
        //next_event = EPOLLIN | EPOLLET;
    }
    else if (command.substr(0, 6) == "mkdir ") { // 创建目录
        //next_event = EPOLLIN | EPOLLET;
    }
    else if (command.substr(0, 6) == "rmdir ") { // 删除目录
        //next_event = EPOLLIN | EPOLLET;
    }
    else if (command.substr(0, 3) == "rm ") { // 删除文件
        //next_event = EPOLLIN | EPOLLET;
    }
    else if (command == "dic") { // 断开通道
        //next_event = EPOLLIN | EPOLLET;
    }
    else {
        std::cerr << "Unknown command: " << command << std::endl;
        return;
    }
}

// for command analyser event, flags = 1
// 其实是上一个函数的延伸
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
        ll_event = new event(llfd, EPOLLIN | EPOLLET, ev->p_rea->event_buf_size);
        if (!ll_event) {
            std::cerr << "Failed to create event for listening socket" << std::endl;
            break;
        }
        ll_event->flags = 2; // 二级监听事件
        if (ll_event->p_rea->add_event(ll_event) == false) {
            break;
        }
        auto ll_func = std::bind(create_data_channel, ll_event);
        ll_event->set(EPOLLIN | EPOLLET, std::move(ll_func));
        ll_event->apply_to_reactor(ev->p_rea);
        ev->remove_from_tree();
        auto func = std::bind(notify_client, ev, ll_event);
        ev->set(EPOLLOUT | EPOLLET, func);
        ev->add_to_tree();
        return;
    } while (0);
    close(llfd);
    if (ll_event) {
        delete ll_event;
        ll_event = nullptr;
    }
}

// for command analyser event, flags = 1
void notify_client(event* ev, event* s_ev) {
    if (!ev || !ev->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return;
    }
    sockaddr_in serv_addr = {0};
    socklen_t addr_len = sizeof(serv_addr);
    if (getsockname(s_ev->fd, (struct sockaddr*)&serv_addr, &addr_len) < 0) {
        std::cerr << "Failed to get socket name: " << strerror(errno) << std::endl;
        return;
    }
    short port = ntohs(serv_addr.sin_port);
    std::string ip = inet_ntoa(serv_addr.sin_addr);
    std::cout << "Data channel created at " << ip << ":" << port << std::endl;
    // 通知客户端数据通道已创建
    std::string resp = "227 Entering Passive Mode ("
                       + ip + "," + std::to_string(port / 256)
                       + "," + std::to_string(port % 256) + ")\n";
    strcpy(ev->buf, resp.c_str());
    ev->buflen = resp.size();
    int datasize = resp.size();
    if (write_size_to(ev, &datasize) <= 0) {
        std::cerr << "Failed to write data size to event" << std::endl;
        return;
    }
    while (datasize > 0) {
        int n = write_to(ev);
        if (n <= 0) {
            std::cerr << "Failed to write data to client" << std::endl;
            return;
        }
        datasize -= n;
    }
    ev->remove_from_tree();
    ev->set(EPOLLIN | EPOLLET, std::bind(command_analyser, ev));
    ev->add_to_tree();
    std::cout << "Client notified about data channel creation" << std::endl;
}

// for sub listener event, flags = 2
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
        nc_event = new event(new_cfd, EPOLLIN | EPOLLET, ev->p_rea->event_buf_size);
        if (!nc_event) {
            std::cerr << "Failed to create event for new connection" << std::endl;
            break;
        }
        nc_event->flags = 3; // 文件传输读/写事件
        if (nc_event->p_rea->add_event(nc_event) == false) {
            break;
        }
        nc_event->apply_to_reactor(ev->p_rea);
        // 设置回调函数
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

