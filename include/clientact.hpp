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

    void send_request(); // PASV command and create data channel
    void close_data_channel();
    void disconnect(); // Disconnect from server

    void send_message(const std::string& msg);
    void recv_message(std::string& msg);

    void print_connection_info();
    void change_dir();
    void list_files();
    void create_dir();
    void remove_dir();
    void upload_file();
    void download_file();
    void delete_file();
};

class Socket {
public:
    int fd = -1;
    std::string ip;
    uint16_t port;
    sa_family_t family = AF_INET;
    struct sockaddr_in serv_addr;
    char *buf = nullptr;
    size_t buflen = 0;
    size_t buffer_size = BUFSIZ;

    Socket() = delete;
    Socket(int fd, std::string ip, uint16_t port, sa_family_t family);
    ~Socket();

    bool connect();
    ssize_t read_size(size_t *datasize);
    ssize_t write_size(size_t* datasize);
    ssize_t sread(size_t& leftsize, size_t& alreadyread);
    ssize_t swrite(size_t& leftsize, size_t& alreadywrite);
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