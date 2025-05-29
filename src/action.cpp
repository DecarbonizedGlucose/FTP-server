#include "../include/action.hpp"

/* ---------- io funcs ---------- */

/*
void recv_data(event* ev) {}

void send_data(event* ev) {}

void test_recv_data(event* e) {
    if (!e || !e->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return;
    }
    int n;
    // Read data from the socket
    while (true) {
        n = read(e->fd, e->buf, e->buffer_size);
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "No data available to read from client: " << e->fd << std::endl;
                break;
            }
            else {
                throw std::runtime_error("Read error: " + std::string(strerror(errno)));
            }
        }
        else if (n == 0) {
            std::cerr << "Client closed connection: " << e->fd << std::endl;
            e->remove_from_tree();
            for (auto& ev : e->p_rea->events) {
                if (ev && ev->fd == e->fd) {
                    ev = nullptr; // Remove from reactor's event list
                    break;
                }
            }
            close(e->fd);
            delete e;
            e = nullptr;
            return;
        }
        e->buf[n] = '\0';
        e->buflen = n;
        std::cout << "Received from client " << e->fd << ": " << e->buf << std::endl;
        // Process data
        for (int i = 0; i < n; ++i) {
            e->buf[i] = toupper(e->buf[i]);
        }
        std::cout << "Processed data: " << e->buf << std::endl;
    }
    auto func = std::bind(test_send_data, e);
    e->remove_from_tree();
    e->set(EPOLLOUT | EPOLLET, std::move(func));
    e->add_to_tree();
}

void test_send_data(event* e) {
    if (!e || !e->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return;
    }
    int n;
again:
    n = write(e->fd, e->buf, e->buflen);
    if (n == -1) {
        if (errno == EINTR) {
            goto again;
        }
        else {
            throw std::runtime_error("Write error: " + std::string(strerror(errno)));
        }
    }
    std::cout << "Sent: " << e->buf << " to client " << e->fd << std::endl;
    auto func = std::bind(test_recv_data, e);
    e->remove_from_tree();
    e->set(EPOLLIN | EPOLLET, std::move(func));
    e->add_to_tree();
}
*/

int read_size_from(event* e, int* datasize) {
    if (!e || !e->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return false;
    }
    int n;
again:
    n = read(e->fd, datasize, sizeof(int));
    if (n == -1) {
        if (errno == EINTR) {
            goto again;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cerr << "No data available to read from client: " << e->fd << std::endl;
            return 0;
        }
        else {
            std::cerr << "Read error: " << strerror(errno) << std::endl;
            return 0;
        }
    }
    else if (n == 0) {
        std::cerr << "Client closed connection: " << e->fd << std::endl;
        return -1;
    }
    else if (n != sizeof(int)) {
        std::cerr << "Read size mismatch: expected " << sizeof(int) << ", got " << n << std::endl;
        return 0;
    }
    return 1;
}

int read_size_from(int fd, int* datasize) {
    if (fd < 0 || !datasize) {
        std::cerr << "Invalid file descriptor or data size pointer" << std::endl;
        return false;
    }
    int n;
again:
    n = read(fd, datasize, sizeof(int));
    if (n == -1) {
        if (errno == EINTR) {
            goto again;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cerr << "No data available to read from fd: " << fd << std::endl;
            return 0;
        }
        else {
            std::cerr << "Read error: " << strerror(errno) << std::endl;
            return 0;
        }
    }
    else if (n == 0) {
        std::cerr << "Connection closed on fd: " << fd << std::endl;
        return -1;
    }
    else if (n != sizeof(int)) {
        std::cerr << "Read size mismatch: expected " << sizeof(int) << ", got " << n << std::endl;
        return 0;
    }
    return 1;
}

int write_size_to(event* e, int* datasize) {
    if (!e || !e->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return 0;
    }
    int n;
again:
    n = write(e->fd, datasize, sizeof(int));
    if (n == -1) {
        if (errno == EINTR) {
            goto again;
        }
        else {
            std::cerr << "Write error: " << strerror(errno) << std::endl;
            return -1;
        }
    }
    else if (n != sizeof(int)) {
        std::cerr << "Write size mismatch: expected " << sizeof(int) << ", got " << n << std::endl;
        return -1;
    }
    return 1;
}

int write_size_to(int fd, int* datasize) {
    int n;
again:
    n = write(fd, datasize, sizeof(int));
    if (n == -1) {
        if (errno == EINTR) {
            goto again;
        }
        else {
            std::cerr << "Write error: " << strerror(errno) << std::endl;
            return -1;
        }
    }
    else if (n != sizeof(int)) {
        std::cerr << "Write size mismatch: expected " << sizeof(int) << ", got " << n << std::endl;
        return -1;
    }
    return 1;
}

int read_from(event* e) {
    if (!e || !e->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return -1;
    }
    int n;
again:
    n = read(e->fd, e->buf, e->buffer_size);
    if (n == -1) {
        if (errno == EINTR) {
            goto again;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cerr << "No data available to read from client: " << e->fd << std::endl;
            return 0;
        }
        else {
            std::cerr << "Read error: " << strerror(errno) << std::endl;
            return -1;
        }
    }
    else if (n == 0) {
        std::cerr << "Client closed connection: " << e->fd << std::endl;
        return -1;
    }
    e->buf[n] = '\0';
    e->buflen = n;
    return n;
}

int read_from(int fd, char* buf, int buf_size, int* buflen) {
    if (fd < 0 || !buf || buf_size <= 0) {
        std::cerr << "Invalid file descriptor or buffer" << std::endl;
        return -1;
    }
    int n;
again:
    n = read(fd, buf, buf_size);
    if (n == -1) {
        if (errno == EINTR) {
            goto again;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cerr << "No data available to read from client: " << fd << std::endl;
            return 0;
        }
        else {
            std::cerr << "Read error: " << strerror(errno) << std::endl;
            return -1;
        }
    }
    else if (n == 0) {
        std::cerr << "Client closed connection: " << fd << std::endl;
        return -1;
    }
    buf[n] = '\0';
    *buflen = n;
    return n;
}

int write_to(event* e) {
    if (!e || !e->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return -1;
    }
    int n;
again:
    n = write(e->fd, e->buf, e->buflen);
    if (n == -1) {
        if (errno == EINTR) {
            goto again;
        }
        else {
            std::cerr << "Write error: " << strerror(errno) << std::endl;
            return -1;
        }
    }
    else if (n != e->buflen) {
        std::cerr << "Write size mismatch: expected " << e->buflen << ", got " << n << std::endl;
        return -1;
    }
    return n;
}

int write_to(int fd, const char* buf, int buf_size, int* buflen) {
    if (fd < 0 || !buf || buf_size <= 0) {
        std::cerr << "Invalid file descriptor or buffer" << std::endl;
        return -1;
    }
    int n;
again:
    n = write(fd, buf, *buflen);
    if (n == -1) {
        if (errno == EINTR) {
            goto again;
        }
        else {
            std::cerr << "Write error: " << strerror(errno) << std::endl;
            return -1;
        }
    }
    else if (n != *buflen) {
        std::cerr << "Write size mismatch: expected " << buflen << ", got " << n << std::endl;
        return -1;
    }
    return n;
}