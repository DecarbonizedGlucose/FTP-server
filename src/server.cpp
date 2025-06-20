#include "../include/serveract.hpp"

int main(int argc, char *argv[]) {
    try {
        init_root_dir(argv[1]);
        reactor rea("", 2100, AF_INET, BUFSIZ, 50, 128, 2000);
        auto pool = new thread_pool(20);
        pool->init();
        rea.add_pool(pool);
        rea.listen_init(root_connection);

        while (true) {
            int nready = rea.wait();
            if (nready < 0) {
                std::cerr << "Error waiting for events: " << strerror(errno) << std::endl;
                break;
            }
            for (int i = 0; i < nready; ++i) {
                event *ev = static_cast<event *>(rea.epoll_events[i].data.ptr);
                if (ev == nullptr) {
                    std::cerr << "Event pointer is null" << std::endl;
                    continue;
                }
                ev->call_back();
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 42;
    }
    return 0;
}