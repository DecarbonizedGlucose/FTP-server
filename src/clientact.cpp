#include "../include/clientact.hpp"
#include <regex>

/* ---------- interface ---------- */

void interface::main_loop() {
    while (running) {
        //system("clear");
        if (!p_client->cntl_connected) {
            un_menu();
            std::cout << "Please enter a command: ";
            std::getline(std::cin, command_num);

            if (command_num == "1") {
                p_client->cntl_connect();
                //system("pause");
            } else if (command_num == "2") {
                running = false;
                break;
            } else {
                std::cout << "Invalid command. Please try again." << std::endl;
            }
        }
        else {
            cd_menu();
            std::cout << "Please enter a command: ";
            std::getline(std::cin, command_num);
            if (command_num == "1") {
                // 切换目录
                this->change_dir();
            }
            else if (command_num == "2") {
                // 远程ls命令
                this->list_files();
            }
            else if (command_num == "3") {
                // 创建目录
                this->create_dir();
            }
            else if (command_num == "4") {
                // 删除目录
                this->remove_dir();
            }
            else if (command_num == "5") {
                // 发送文件管理请求
                this->send_request();
            }
            else if (command_num == "6") {
                // Call upload file function here
            }
            else if (command_num == "7") {
                // Call download file function here
            }
            else if (command_num == "8") {
                // 删除文件
                this->delete_file();
            }
            else if (command_num == "9") {
                // 关闭数据通道
                this->close_data_channel();
            }
            else if (command_num == "0") {
                // 断开连接
                this->disconnect();
            }
            else {
                std::cout << "Invalid command. Please try again." << std::endl;
            }
        }
    }
}

void interface::un_menu() {
    std::cout << "=============================================" << std::endl;
    std::cout << "             Welcome to FTP Client"            << std::endl;
    std::cout                                                    << std::endl;
    std::cout << "             1. Connect to server"             << std::endl;
    std::cout << "             2.       Exit"                    << std::endl;
    std::cout                                                    << std::endl;
    print_connection_info();
    std::cout << "=============================================" << std::endl;
}

void interface::cd_menu() {
    std::cout << "=============================================" << std::endl;
    std::cout                                                    << std::endl;
    std::cout << "             1. Change Directory"              << std::endl;
    std::cout << "             2. List Files"                    << std::endl;
    std::cout << "             3. Create Directory"              << std::endl;
    std::cout << "             4. Send PASV Command"             << std::endl;
    std::cout << "             5. Upload File"                   << std::endl;
    std::cout << "             6. Download File"                 << std::endl;
    std::cout << "             7. Delete File"                   << std::endl;
    std::cout << "             8. Close Data Channel"            << std::endl;
    std::cout << "             9. Disconnect"                    << std::endl;
    std::cout                                                    << std::endl;
    print_connection_info();
    std::cout << "=============================================" << std::endl;
}

void interface::print_connection_info() {
    if (p_client->cntl_connected) {
        std::cout << "Control Connection: " << p_client->cntl_socket->ip << ":"
                  << p_client->cntl_socket->port << std::endl;
    } else {
        std::cout << "No control connection established." << std::endl;
        return;
    }
    if (p_client->data_connected) {
        std::cout << "Data Connection: " << p_client->data_socket->ip << ":"
                  << p_client->data_socket->port << std::endl;
    } else {
        std::cout << "No data connection established." << std::endl;
    }
}

void interface::change_dir() {
    std::string dir_path, resp;
    std::cout << "Enter directory path: ";
    std::getline(std::cin, dir_path);
    if (dir_path.empty()) {
        std::cerr << "Directory path cannot be empty." << std::endl;
        return;
    }
    send_message("cd " + dir_path);
    recv_message(resp);
    if (resp.empty()) {
        std::cerr << "Failed to get response." << std::endl;
        return;
    }
    std::cout << resp;
}

void interface::list_files() {
    std::string resp = "ls";
    send_message(resp);
    recv_message(resp);
    if (resp.empty()) {
        std::cerr << "Failed to get response." << std::endl;
        return;
    }
    std::cout << resp;
}

void interface::create_dir() {
    std::string dir_name, resp;
    std::cout << "Enter new directory name: ";
    std::getline(std::cin, dir_name);
    if (dir_name.empty()) {
        std::cerr << "Directory name cannot be empty." << std::endl;
        return;
    }
    send_message("mkdir " + dir_name);
    recv_message(resp);
    if (resp.empty()) {
        std::cerr << "Failed to get response." << std::endl;
        return;
    }
    std::cout << resp;
}

void interface::remove_dir() {
    std::string dir_to_del, resp;
    std::cout << "Enter the directory name to delete: ";
    std::getline(std::cin, dir_to_del);
    if (dir_to_del.empty()) {
        std::cerr << "Directory name cannot be empty." << std::endl;
        return;
    }
    send_message("rmdir " + dir_to_del);
    recv_message(resp);
    if (resp.empty()) {
        std::cerr << "Failed to get response." << std::endl;
        return;
    }
    std::cout << resp;
}

void interface::send_request() {
    if (!p_client->cntl_connected) {
        std::cerr << "Control connection not established. Please connect first." << std::endl;
        return;
    }
    if (p_client->data_connected) {
        std::cerr << "Data channel already created." << std::endl;
        return;
    }
    // 发送 PASV 命令
    std::string msg = "PASV", resp;
    send_message(msg);
    // 读取 PASV 响应
    recv_message(resp);
    std::cout << "PASV response: " << msg << std::endl;
    uint16_t port, p1, p2;
    std::regex regex("227 Entering Passive Mode \\((\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)\\)");
    std::smatch match;
    if (std::regex_search(resp, match, regex) && match.size() == 7) {
        p1 = std::stoi(match[5].str());
        p2 = std::stoi(match[6].str());
    } else {
        std::cerr << "Failed to parse PASV response." << std::endl;
        return;
    }
    if (p1 < 0 || p1 > 255 || p2 < 0 || p2 > 255) {
        std::cerr << "Invalid port numbers received from server." << std::endl;
        return;
    }
    port = p1 * 256 + p2;
    std::cout << "Data channel addr: " << p_client->cntl_socket->ip << ":" << port << std::endl;
    if (port <= 0) {
        std::cerr << "Invalid port number received from server." << std::endl;
        return;
    }
    // 连接数据通道
    if (!p_client->data_connect(port)) {
        std::cerr << "Failed to connect to data channel." << std::endl;
        return;
    }
    printf("227 Entering Passive Mode (%s:%d)\n",
           p_client->data_socket->ip.c_str(), p_client->data_socket->port);
           // 这后面改成那个形式
    p_client->data_connected = true;
}

void interface::upload_file() {
    if (!p_client->data_connected) {
        std::cerr << "Data channel not created." << std::endl;
        return;
    }
    // Implement file upload logic here
}

void interface::download_file() {
    if (!p_client->data_connected) {
        std::cerr << "Data channel not created." << std::endl;
        return;
    }
    // Implement file download logic here
}

void interface::delete_file() {
    if (!p_client->data_connected) {
        std::cerr << "Data channel not created." << std::endl;
        return;
    }
    std::string file_to_del, resp;
    std::cout << "Enter the file name to delete: ";
    std::getline(std::cin, file_to_del);
    if (file_to_del.empty()) {
        std::cerr << "File name cannot be empty." << std::endl;
        return;
    }
    send_message("remove" + file_to_del);
    recv_message(resp);
    if (resp.empty()) {
        std::cerr << "Failed to get response." << std::endl;
        return;
    }
    std::cout << resp;
}

void interface::close_data_channel() {
    // 检测数据通道存在性
    if (!p_client->data_connected) {
        std::cerr << "No data channel to close." << std::endl;
        return;
    }
    // 通知服务器关闭数据通道
    std::string msg = "close";
    send_message(msg);
    // 本地关闭数据通道
    p_client->data_disconnect();
    std::cout << "Data channel closed." << std::endl;
}

void interface::disconnect() {
    // 检测控制连接存在性
    if (!p_client->cntl_connected) {
        std::cerr << "No control connection to disconnect." << std::endl;
        return;
    }
    // 发送断开连接命令
    std::string msg = "dic";
    send_message(msg);
    // 本地断开控制连接
    p_client->cntl_disconnect();
    std::cout << "Disconnected from server." << std::endl;
    running = false; // 退出主循环
}

void interface::send_message(const std::string& msg) {
    strcpy(p_client->cntl_socket->buf, msg.c_str());
    int len = p_client->cntl_socket->buflen = strlen(p_client->cntl_socket->buf);
    int al = 0, n;
    do {
        n = p_client->cntl_socket->write_size(&len);
        if (n < 0) {
            std::cerr << "Failed to write size to control socket." << std::endl;
            return;
        }
    } while (n == 0);
    do {
        n = p_client->cntl_socket->swrite(len, al);
        if (n < 0) {
            std::cerr << "Failed to send PASV command." << std::endl;
            return;
        }
    } while (len || n == 0);
}

void interface::recv_message(std::string& msg) {
    int len, n, al = 0;
    do {
        n = p_client->cntl_socket->read_size(&len);
        if (n < 0) {
            std::cerr << "Failed to read response size from server." << std::endl;
            return;
        }
    } while (n == 0);
    p_client->cntl_socket->buflen = len;
    al = 0;
    if (len <= 0) {
        std::cerr << "Failed to read response from server." << std::endl;
        return;
    }
    do {
        n = p_client->cntl_socket->sread(len, al);
        if (n < 0) {
            std::cerr << "Failed to read PASV response." << std::endl;
            return;
        }
    } while (len || n == 0);
    p_client->cntl_socket->buf[p_client->cntl_socket->buflen] = '\0';
    msg.assign(p_client->cntl_socket->buf, p_client->cntl_socket->buflen);
}

/* ---------- socket ---------- */

Socket::Socket(int fd, std::string ip, uint16_t port, sa_family_t family = AF_INET)
    : fd(fd), ip(ip), port(port), family(family) {
    if (fd < 0) {
        throw std::invalid_argument("Invalid file descriptor - " + std::string(strerror(errno)));
    }
    serv_addr.sin_family = family;
    serv_addr.sin_port = htons(port);
    if (inet_pton(family, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid IP address - " + std::string(strerror(errno)));
    }
    buf = new char[buffer_size + 1];
    if (!buf) {
        throw std::runtime_error("Memory allocation failed for socket buffer - " + std::string(strerror(errno)));
    }
    buf[buffer_size] = '\0';
}

Socket::~Socket() {
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
    if (buf) {
        delete[] buf;
        buf = nullptr;
    }
    buflen = 0;
}

bool Socket::connect() {
    if (fd < 0) {
        return false;
    }
    if (::connect(fd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0) {
        return false;
    }
    return true;
}

int Socket::read_size(int* datasize) {
    int ret;
    do {
        ret = read_size_from(fd, datasize);
    } while (ret == 0);
    return ret;
}

int Socket::write_size(int* datasize) {
    int ret;
    do {
        ret =  write_size_to(fd, datasize);
    } while (ret == 0);
    return ret;
}

int Socket::sread(int& leftsize, int& alreadyread) {
    if (leftsize <= 0 || alreadyread < 0 || alreadyread > buflen) {
        std::cerr << "Invalid parameters for sread." << std::endl;
        return -1;
    }
    int n;
    do {
        n = read_from(fd, buf + alreadyread, buffer_size, &buflen);
    } while (n == 0);
    if (n < 0) {
        std::cerr << "Read error in Socket::sread: " << strerror(errno) << std::endl;
        return -1;
    }
    leftsize -= n;
    alreadyread += n;
    return n;
}

int Socket::swrite(int& leftsize, int& alreadywrite) {
    if (leftsize <= 0 || alreadywrite < 0 || alreadywrite >= buflen) {
        std::cerr << "Invalid parameters for swrite." << std::endl;
        return -1;
    }
    int n;
    do {
        n = write_to(fd, buf + alreadywrite, buflen, &buflen);
    } while (n == 0);
    if (n < 0) {
        return n;
    }
    leftsize -= n;
    alreadywrite += n;
    return n;
}

/* ---------- client ---------- */

ftp_client::ftp_client() {
    pool = new thread_pool(4, 8);
    pool->init();
    std::string home_dir = std::string(getenv("HOME"));
    fm = new file_manager(home_dir);
    ui = new interface(this);
}

ftp_client::~ftp_client() {
    ui->running = false;
    if (pool) {
        pool->shutdown();
        delete pool;
        pool = nullptr;
    }
    if (fm) {
        delete fm;
        fm = nullptr;
    }
    if (cntl_socket) {
        delete cntl_socket;
        cntl_socket = nullptr;
    }
    if (data_socket) {
        delete data_socket;
        data_socket = nullptr;
    }
    cntl_connected = false;
    data_connected = false;
}

void ftp_client::launch() {
    pool->init();
    ui->running = true;
    ui->main_loop();
    pool->shutdown();
}

void ftp_client::cntl_connect() {
    std::cout << "Type in the server's ip address: ";
    std::string server_ip;
    std::getline(std::cin, server_ip);
    std::cout << "You typed: " << '"' << server_ip << '"' << std::endl;
    if (server_ip.empty()) {
        std::cerr << "Invalid IP address." << std::endl;
        return;
    }
    cntl_socket = new Socket(socket(AF_INET, SOCK_STREAM, 0), server_ip, 2100);
    if (!cntl_socket->connect()) {
        std::cerr << "Failed to connect to server: " << server_ip << ":" << 2100 << std::endl;
        delete cntl_socket;
        cntl_socket = nullptr;
        return;
    }
    cntl_connected = true;
    std::cout << "Connected to server: " << server_ip << ":" << 2100 << std::endl;
}

void ftp_client::cntl_disconnect() {
    if (data_connected) {
        data_disconnect();
    }
    if (cntl_connected) {
        delete cntl_socket;
        cntl_socket = nullptr;
        cntl_connected = false;
        std::cout << "Disconnected from control socket." << std::endl;
    } else {
        std::cerr << "No control connection to disconnect." << std::endl;
    }
}

bool ftp_client::data_connect(uint16_t port) {
    if (!cntl_connected) {
        std::cerr << "Control connection not established. Please connect first." << std::endl;
        return false;
    }
    if (data_connected) {
        std::cerr << "Data connection already established." << std::endl;
        return false;
    }
    data_socket = new Socket(socket(AF_INET, SOCK_STREAM, 0), cntl_socket->ip, port);
    if (!data_socket->connect()) {
        std::cerr << "Failed to connect to data socket: " << data_socket->ip << ":" << port << std::endl;
        delete data_socket;
        data_socket = nullptr;
        return false;
    }
    data_connected = true;
    std::cout << "Data connection established: " << data_socket->ip << ":" << port << std::endl;
    return true;
}

void ftp_client::data_disconnect() {
    if (data_socket) {
        if (data_connected) {
            std::cout << "Disconnecting from data socket..." << std::endl;
            data_connected = false;
            delete data_socket;
            data_socket = nullptr;
        } else {
            std::cerr << "No data connection to disconnect." << std::endl;
            return;
        }
    }
}
