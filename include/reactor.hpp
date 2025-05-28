#pragma once
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

namespace net {
    const int max_open_files = 1024;
}

class event;
class reactor;

class reactor {
private:
    std::string ip = "";
    sa_family_t family = AF_INET;
    short port = 8080;
    int epoll_fd;
    int listen_fd;
public:
    friend class event;
    std::vector<event*> events;
    struct epoll_event* epoll_events = nullptr;
    int max_events = 50;
    int max_clients = 128;
    int epoll_timeout = 2000; // milliseconds
    int event_buf_size = BUFSIZ;

    reactor();
    reactor(std::string ip, short port, sa_family_t fam,
            int buf_size, int max_events, int max_clnts, int timeout);
    reactor(const reactor&) = delete;
    reactor(reactor&&) = delete;
    reactor& operator=(const reactor&) = delete;
    reactor& operator=(reactor&&) = delete;
    ~reactor();

    void listen_init();
    int wait();
};

class event {
public:
    reactor* p_rea;
    int fd = -1;
    int events = 0;
    bool on_tree = false;
    char* buf = nullptr;
    int buflen = 0;
    int buffer_size = BUFSIZ;
    time_t last_active;
    std::function<void()> call_back_func = nullptr;

    event() = delete;
    event(int fd, int events, int buffer_size, std::function<void()> call_back_func);
    event(int fd, int events, int buffer_size);
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
};

void recv_data(event* ev);

void send_data(event* ev);

void accept_connection(event* ev);

void test_recv_data(event* e);
void test_send_data(event* e);