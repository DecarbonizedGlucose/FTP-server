#include "reactor.hpp"
#include "thread_pool.hpp"
#include "ftp.hpp"
#include <iostream>
#include <stdexcept>

int main() {
    reactor control_reactor("", 2100, AF_INET, BUFSIZ, 50, 128, 1000);
    control_reactor.listen_init();
    thread_pool pool(8, 20);
    pool.init();

    
}