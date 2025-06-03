#ifndef REACTOR_HPP
#define REACTOR_HPP

#include <ctime>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <functional>
#include <memory>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <any>

#include "../include/thread_pool.hpp"

// namespace net {
//     const int max_open_files = 1024;
// }

class event;
class reactor;

class reactor {
private:
    std::string ip = "";
    sa_family_t family = AF_INET;
    uint16_t port = 0;
    int epoll_fd;
    int listen_fd;
public:
    friend class event;
    std::unordered_map<int, event*> events; // 使用unordered_map来存储事件
    struct epoll_event* epoll_events = nullptr;
    int max_events = 50;
    int max_clients = 128;
    int epoll_timeout = 2000; // milliseconds
    int event_buf_size = BUFSIZ;

    thread_pool* pool = nullptr;

    reactor();
    reactor(std::string ip, uint16_t port, sa_family_t fam,
            size_t buf_size, int max_events, int max_clnts, int timeout);
    reactor(const reactor&) = delete;
    reactor(reactor&&) = delete;
    reactor& operator=(const reactor&) = delete;
    reactor& operator=(reactor&&) = delete;
    ~reactor();

    void add_pool(thread_pool* p);
    void listen_init(void (*root_connection)(event*));
    int wait();
    bool add_event(event* ev);
    bool remove_event(event* ev);

    int get_listen_fd() const { return listen_fd; }
    int get_epoll_fd() const { return epoll_fd; }
};

class event {
public:
    reactor* p_rea;
    int fd = -1;
    int events = 0;
    bool on_tree = false;
    char* buf = nullptr;
    size_t buflen = 0;
    size_t buffer_size = BUFSIZ;
    time_t last_active;
    std::function<void()> call_back_func = nullptr;

    // 为本项目定制的成员
    int flags = 0; // 用来标记事件的层级(好像除了给人看没啥用)
    std::any data; // 你不能什么都往这里塞
    std::any cntler;

    event() = delete;
    event(int fd, int events, size_t buffer_size, std::function<void()> call_back_func);
    event(int fd, int events, size_t buffer_size);
    event(const event&) = delete;
    event(event&&) = delete;
    event& operator=(const event&) = delete;
    event& operator=(event&&) = delete;
    ~event();

    void set(int new_events, std::function<void()> call_back_func);
    void set(int new_events);
    void set(std::function<void()> call_back_func);
    void apply_to_reactor(reactor* rea);
    bool in_reactor() const;
    void remove_from_tree();
    void add_to_tree();
    bool is_buf_full() const;
    void call_back();

    void send_message(const std::string& msg);
    void recv_message(std::string& msg);
};

#endif