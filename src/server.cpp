#include "../include/serveract.hpp"


int main() {
    reactor rea("", 2100, AF_INET, BUFSIZ, 50, 128, 2000);
    thread_pool pool(8);
    pool.init();
    rea.listen_init(root_connection);

    int nready;
    event* l_event = rea.events.back();

    while (true) {
        nready = rea.wait();
        if (nready < 0) {
            std::cerr << "Error waiting for events: " << strerror(errno) << std::endl;
            break;
        }
        for (int i = 0; i < nready; ++i) {
            event* ev = static_cast<event*>(rea.epoll_events[i].data.ptr);
            if (ev == l_event && ev->events & EPOLLIN) {
                // Handle read event
                ev->call_back_func();
            }
        }
    }

    return 0;
}