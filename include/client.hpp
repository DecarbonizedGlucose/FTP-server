#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>
#include "../include/thread_pool.hpp"
#include "../include/action.hpp"

class Socket;
class interface;
class ftp_client;
class file_manager;

class interface {
public:
    ftp_client* p_client;
    std::string command_num;
    std::string control_info;
    std::string data_info;
    bool running;

    interface(ftp_client* client) : p_client(client), running(false) {}
    ~interface() = default;
    void main_loop();
    void un_menu();
    void cd_menu();

    void print_connection_info();
    void change_dir(const std::string& dir_path);
    void list_files();
    void create_dir(const std::string& dir_name);
    void send_request(const std::string& command);
    void upload_file(const std::string& file_path);
    void download_file(const std::string& file_path);
    void delete_file(const std::string& file_path);
};

class file_manager {};

class Socket {
public:
    int fd = -1;
    std::string ip;
    short port;
    sa_family_t family = AF_INET;
    struct sockaddr_in serv_addr;
    char *buf = nullptr;
    int buflen = 0;
    int buffer_size = BUFSIZ;

    Socket() = delete;
    Socket(int fd, std::string ip, short port, sa_family_t family);
    ~Socket();

    bool connect();
};

class ftp_client {
private:
    friend class interface;
    thread_pool* pool = nullptr;
    interface ui = interface(this);
    file_manager* fm = nullptr;
    Socket* conn_socket = nullptr;
    Socket* data_socket = nullptr;
public:
    bool cntl_connected = false;
    bool data_connected = false;

    ftp_client();
    ~ftp_client();
    void launch();
    void cntl_connect();
    void data_connect();
    void cntl_disconnect();
    void data_disconnect();
};

/* ---------- interface ---------- */

void interface::main_loop() {
    while (running) {
        system("clear");
        if (!p_client->cntl_connected) {
            un_menu();
            std::cout << "Please enter a command: ";
            std::getline(std::cin, command_num);

            if (command_num == "1") {
                p_client->cntl_connect();
                system("pause");
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
                std::cout << "Enter directory path: ";
                std::string dir_path;
                std::getline(std::cin, dir_path);
            }
            else if (command_num == "2") {
                // 远程ls命令
            }
            else if (command_num == "3") {
                // 创建目录
                std::cout << "Enter new directory name: ";
                std::string dir_name;
                std::getline(std::cin, dir_name);
            }
            else if (command_num == "4") {
                // 发送文件管理请求
                p_client->data_connect();
                system("pause");
            }
            else if (command_num == "5") {
                // Call upload file function here
            }
            else if (command_num == "6") {
                // Call download file function here
            }
            else if (command_num == "7") {
                // Call delete file function here
            }
            else if (command_num == "8") {
                p_client->data_disconnect();
            }
            else if (command_num == "9") {
                p_client->cntl_disconnect();
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
    std::cout << "             Connected to FTP Server"          << std::endl;
    std::cout                                                    << std::endl;
    std::cout << "             1. Change Directory"              << std::endl;
    std::cout << "             2. List Files"                    << std::endl;
    std::cout << "             3. Create Directory"              << std::endl;
    std::cout << "             4. Send PASV Command"             << std::endl;
    std::cout << "             5. Upload File"                   << std::endl;
    std::cout << "             6. Download File"                 << std::endl;
    std::cout << "             7. Delete File"                   << std::endl;
    std::cout << "             8. Stop Managing Files"           << std::endl;
    std::cout << "             9. Disconnect"                    << std::endl;
    std::cout                                                    << std::endl;
    print_connection_info();
    std::cout << "=============================================" << std::endl;
}

void interface::print_connection_info() {
    if (p_client->cntl_connected) {
        std::cout << "Control Connection: " << p_client->conn_socket->ip << ":"
                  << p_client->conn_socket->port << std::endl;
    } else {
        std::cout << "No control connection established." << std::endl;
    }
}

void interface::change_dir(const std::string& dir_path) {

}

void interface::list_files() {

}

void interface::create_dir(const std::string& dir_name) {

}

void interface::send_request(const std::string& command) {

}

void interface::upload_file(const std::string& file_path) {
    if (!p_client->data_connected) {
        std::cerr << "Data channel not created." << std::endl;
        return;
    }
    // Implement file upload logic here
}

void interface::download_file(const std::string& file_path) {
    if (!p_client->data_connected) {
        std::cerr << "Data channel not created." << std::endl;
        return;
    }
    // Implement file download logic here
}

void interface::delete_file(const std::string& file_path) {
    if (!p_client->data_connected) {
        std::cerr << "Data channel not created." << std::endl;
        return;
    }
    // Implement file deletion logic here
}

/* ---------- socket ---------- */

Socket::Socket(int fd, std::string ip, short port, sa_family_t family = AF_INET)
    : fd(fd), ip(std::move(ip)), port(port), family(family) {
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

/* ---------- client ---------- */

ftp_client::ftp_client() {
    pool = new thread_pool(4, 8);
    pool->init();
    fm = new file_manager();
}

ftp_client::~ftp_client() {
}

void ftp_client::launch() {
    pool->init();
    ui.running = true;
    ui.main_loop();
    pool->shutdown();
}

void ftp_client::cntl_connect() {
    std::cout << "Type in the server's ip address: ";
    std::string server_ip;
    std::getline(std::cin, server_ip);
    if (server_ip.empty()) {
        std::cerr << "Invalid IP address or port." << std::endl;
        return;
    }
    conn_socket = new Socket(socket(AF_INET, SOCK_STREAM, 0), server_ip, 2100);
    if (!conn_socket->connect()) {
        std::cerr << "Failed to connect to server: " << server_ip << ":" << 2100 << std::endl;
        delete conn_socket;
        conn_socket = nullptr;
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
        delete conn_socket;
        conn_socket = nullptr;
        cntl_connected = false;
        std::cout << "Disconnected from control socket." << std::endl;
    } else {
        std::cerr << "No control connection to disconnect." << std::endl;
    }
}

void ftp_client::data_connect() {
    if (!cntl_connected) {
        std::cerr << "Control connection not established. Please connect first." << std::endl;
        return;
    }
    if (data_connected) {
        std::cerr << "Data connection already established." << std::endl;
        return;
    }
    data_socket = new Socket(socket(AF_INET, SOCK_STREAM, 0), conn_socket->ip, 2101);
    if (!data_socket->connect()) {
        std::cerr << "Failed to connect to data socket: " << data_socket->ip << ":" << 2101 << std::endl;
        delete data_socket;
        data_socket = nullptr;
        return;
    }
    data_connected = true;
    std::cout << "Data connection established: " << data_socket->ip << ":" << 2101 << std::endl;
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

#endif