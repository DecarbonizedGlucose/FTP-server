#pragma once
#include <string>
#include <functional>
#include <deque>
#include <list>
#include "../include/reactor.hpp"
#include "../include/thread_pool.hpp"

class interface;
class file;
class file_manager;
class Socket;
class ftp_server;
class ftp_client;

class interface {
private:
    ftp_client* p_client = nullptr;
    std::string command_num;
    std::string control_info;
    std::string data_info;
public:
    bool running;

    interface(ftp_client* p) : p_client(p) {}
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

class file {
public:
    std::string name;
    std::string path;
    bool is_directory = false;
};

class file_manager {
private:
    std::string current_path;
    std::deque<file> files;
};

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

class ftp_server {
private:
    reactor rea;
    thread_pool pool = thread_pool(4, 8);
};

class ftp_client {
private:
    friend class interface;
    thread_pool pool = thread_pool(4, 8);
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