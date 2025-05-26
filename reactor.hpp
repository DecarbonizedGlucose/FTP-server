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
#include <cctype>
#include <string>
#include <functional>
#include <memory>
#include <iostream>

namespace net {
    const int max_open_files = 1024;
}

class reactor {
private:
    std::string ip;
    sa_family_t family;
    short port;
    int event_buf_size;
    int epoll_fd;
    int listen_fd;
    int max_events;
    int max_clients;
    int epoll_timeout;
    class event;
    event* events = nullptr;
    struct epoll_event* epoll_events = nullptr;
    bool running = false;
public:
    reactor() = delete;
    reactor(std::string ip, short port, sa_family_t fam,
            int buf_size, int max_events, int max_clnts, int timeout);
    reactor(const reactor&) = delete;
    reactor(reactor&&) = delete;
    reactor& operator=(const reactor&) = delete;
    reactor& operator=(reactor&&) = delete;
    ~reactor();
};

class reactor::event {
private:
    int fd = -1;
    int events = 0;
    bool in_reactor = false;
    char* buf = nullptr;
    int buflen = 0;
    int buffer_size;
    time_t last_active;

};