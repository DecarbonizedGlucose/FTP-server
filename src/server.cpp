#include "../include/serveract.hpp"


int main() {
    reactor rea("", 2100, AF_INET, BUFSIZ, 50, 128, 2000);
    thread_pool* pool = new thread_pool(8);
    pool->init();
    rea.add_pool(pool);
    rea.listen_init(root_connection);

    int nready;
    event* ev;

    while (true) {
        nready = rea.wait();
        if (nready < 0) {
            std::cerr << "Error waiting for events: " << strerror(errno) << std::endl;
            break;
        }
        for (int i = 0; i < nready; ++i) {
            ev = static_cast<event*>(rea.epoll_events[i].data.ptr);
            if (ev == nullptr) {
                std::cerr << "Event pointer is null" << std::endl;
                continue;
            }
            ev->call_back();
        }
    }

    return 0;
}