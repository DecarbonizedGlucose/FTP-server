#include <iostream>
#include <functional>
#include "reactor.hpp"
#include <cctype>

int main() {
    reactor r = reactor();
    r.listen_init();

    while (1) {
        int nready = r.wait();
        if (!nready) {
            continue;
        }
        for (int i = 0; i < nready; ++i) {
            event* e = static_cast<event*>(r.epoll_events[i].data.ptr);
            if (e && e->events & EPOLLIN) {
                try {
                    e->call_back();
                } catch (const std::exception& ex) {
                    std::cerr << "Error in event callback: " << ex.what() << std::endl;
                }
            }
            if (e && e->events & EPOLLOUT) {
                try {
                    e->call_back();
                } catch (const std::exception& ex) {
                    std::cerr << "Error in event callback: " << ex.what() << std::endl;
                }
            }
        }
    }

    return 0;
}