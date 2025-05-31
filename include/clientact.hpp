#ifndef CLIENTACT_HPP
#define CLIENTACT_HPP

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
    void send_request(); // PASV command
    void upload_file(const std::string& file_path);
    void download_file(const std::string& file_path);
    void delete_file(const std::string& file_path);
};

class file_manager {};

class Socket {
public:
    int fd = -1;
    std::string ip;
    uint16_t port;
    sa_family_t family = AF_INET;
    struct sockaddr_in serv_addr;
    char *buf = nullptr;
    int buflen = 0;
    int buffer_size = BUFSIZ;

    Socket() = delete;
    Socket(int fd, std::string ip, uint16_t port, sa_family_t family);
    ~Socket();

    bool connect();
    int read_size(int *datasize);
    int write_size(int* datasize);
    int sread(int& leftsize, int& alreadyread);
    int swrite(int& leftsize, int& alreadywrite);
};

class ftp_client {
private:
    friend class interface;
    thread_pool* pool = nullptr;
    interface* ui = nullptr;
    file_manager* fm = nullptr;
    Socket* cntl_socket = nullptr;
    Socket* data_socket = nullptr;
public:
    bool cntl_connected = false;
    bool data_connected = false;

    ftp_client();
    ~ftp_client();
    void launch();
    void cntl_connect();
    bool data_connect(uint16_t port);
    void cntl_disconnect();
    void data_disconnect();
};

#endif