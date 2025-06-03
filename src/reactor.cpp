#include "../include/reactor.hpp"
#include "../include/action.hpp"

/* ---------- event ---------- */

event::event(int fd, int events, size_t buffer_size, std::function<void()> call_back_func)
    : p_rea(nullptr), fd(fd), events(events), buffer_size(buffer_size) {
    if (fd < 0 || buffer_size <= 0) {
        throw std::invalid_argument("Invalid file descriptor or buffer size - " + std::string(strerror(errno)));
    }
    buf = new char[buffer_size + 1];
    if (!buf) {
        throw std::runtime_error("Memory allocation failed for event buffer - " + std::string(strerror(errno)));
    }
    buf[buffer_size] = '\0';
}

event::event(int fd, int events, size_t buffer_size)
    : p_rea(nullptr), fd(fd), events(events) {
    if (fd < 0 || buffer_size <= 0) {
        throw std::invalid_argument("Invalid file descriptor or buffer size - " + std::string(strerror(errno)));
    }
    buf = new char[buffer_size + 1];
    if (!buf) {
        throw std::runtime_error("Memory allocation failed for event buffer - " + std::string(strerror(errno)));
    }
    buf[buffer_size] = '\0';
}

event::~event() {
    // 在进行这些之前要把data和cntler的内容清空
    // 手动清理资源
    if (data.has_value() || cntler.has_value()) {
        data.reset();
        cntler.reset();
    }
    if (fd > 0) {
        close(fd);
        fd = -1;
    }
    if (buf) {
        delete[] buf;
        buf = nullptr;
    }
}

void event::set(int new_events, std::function<void()> call_back_func) {
    this->events = new_events;
    if (call_back_func) {
        this->call_back_func = call_back_func;
    } else {
        std::cerr << "Warning: Callback function is null, setting to nullptr." << std::endl;
        this->call_back_func = nullptr;
    }
}

void event::set(int new_events) {
    this->events = new_events;
}

void event::set(std::function<void()> call_back_func) {
    if (call_back_func) {
        this->call_back_func = call_back_func;
    } else {
        std::cerr << "Warning: Callback function is null, setting to nullptr." << std::endl;
        this->call_back_func = nullptr;
    }
}

void event::apply_to_reactor(reactor* rea) {
    // auto added to tree
    if (this->fd < 0) {
        throw std::invalid_argument("File descriptor is invalid - " + std::string(strerror(errno)));
    }
    if (this->p_rea) {
        throw std::runtime_error("Event is already associated with a reactor - " + std::string(strerror(errno)));
    }
    if (this->on_tree) {
        throw std::runtime_error("Event is already in the reactor tree - " + std::string(strerror(errno)));
    }
    this->on_tree = true;
    if (!rea) {
        throw std::invalid_argument("Reactor pointer is null - " + std::string(strerror(errno)));
    }
    this->p_rea = rea;
    struct epoll_event ev = {0, {0}};
    ev.events = this->events;
    ev.data.ptr = this;
    if (epoll_ctl(this->p_rea->epoll_fd, EPOLL_CTL_ADD, this->fd, &ev) < 0) {
        throw std::runtime_error("event::apply_to_reactor: Failed to add event in epoll - " + std::string(strerror(errno)));
    }
    this->last_active = time(nullptr);
}

bool event::in_reactor() const {
    return p_rea != nullptr;
}

void event::remove_from_tree() {
    if (!in_reactor()) {
        throw std::runtime_error("Event is not in a reactor");
    }
    if (!on_tree) {
        return;
    }
    if (epoll_ctl(this->p_rea->epoll_fd, EPOLL_CTL_DEL, this->fd, nullptr) < 0) {
        throw std::runtime_error("Failed to remove event from epoll");
    }
    this->on_tree = false;
}

void event::add_to_tree() {
    if (!in_reactor()) {
        throw std::runtime_error("Event is not associated with a reactor - " + std::string(strerror(errno)));
    }
    struct epoll_event ev = {0, {0}};
    ev.events = this->events;
    ev.data.ptr = this;
    if (epoll_ctl(this->p_rea->epoll_fd, EPOLL_CTL_ADD, this->fd, &ev) < 0) {
        throw std::runtime_error("Failed to modify event in epoll - " + std::string(strerror(errno)));
    }
    this->last_active = time(nullptr);
    this->on_tree = true;
}

bool event::is_buf_full() const {
    return buflen >= buffer_size;
}

void event::call_back() {
    if (call_back_func) {
        if (this->p_rea->pool) {
            this->p_rea->pool->submit(call_back_func);
        } else {
            call_back_func();
        }
    } else {
        std::cerr << "No callback function set for event on fd: " << fd << std::endl;
    }
}

/* ---------- reactor ---------- */

reactor::reactor() {
    this->epoll_fd = epoll_create1(0);
    if (this->epoll_fd < 0) {
        throw std::runtime_error("reactor::reactor: Failed to create epoll instance - " + std::string(strerror(errno)));
    }
    this->listen_fd = -1;
    //this->events.resize(this->max_events + 1, nullptr);
    this->epoll_events = new struct epoll_event[max_events];
    if (!this->epoll_events) {
        throw std::runtime_error("Memory allocation failed for epoll events array - " + std::string(strerror(errno)));
    }
}

reactor::reactor(
    std::string ip, uint16_t port, sa_family_t fam,
    size_t buf_size, int max_events,
    int max_clients, int epoll_timeout
)   : ip(ip), family(fam), port(port),
    event_buf_size(buf_size), max_events(max_events),
    max_clients(max_clients), epoll_timeout(epoll_timeout) {
    this->epoll_fd = epoll_create1(0);
    if (this->epoll_fd < 0) {
        throw std::runtime_error("reactor::reactor: Failed to create epoll instance - " + std::string(strerror(errno)));
    }
    this->listen_fd = -1;
    //this->events.resize(this->max_events + 1, nullptr);
    this->epoll_events = new struct epoll_event[max_events];
    if (!this->epoll_events) {
        throw std::runtime_error("Memory allocation failed for epoll events array - " + std::string(strerror(errno)));
    }
}

reactor::~reactor() {
    if (this->epoll_events) {
        delete[] this->epoll_events;
        this->epoll_events = nullptr;
    }
    for (auto pe : this->events) {
        delete pe.second;
        pe.second = nullptr;
    }
    this->epoll_fd = -1;
}

void reactor::add_pool(thread_pool* p) {
    if (p == nullptr) {
        throw std::invalid_argument("Thread pool pointer is null - " + std::string(strerror(errno)));
    }
    this->pool = p;
}

void reactor::listen_init(void (*root_connection)(event*)) {
    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = this->family;
    serv_addr.sin_port = htons(this->port);
    serv_addr.sin_addr.s_addr = this->ip.empty() ?
                                INADDR_ANY : inet_addr(this->ip.c_str());
    do {
        this->listen_fd = socket(this->family, SOCK_STREAM, 0);
        if (listen_fd < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            break;
        }
        int flag = fcntl(listen_fd, F_SETFL, O_NONBLOCK);
        if (flag < 0) {
            std::cerr << "Failed to set non-blocking mode" << std::endl;
            break;
        }
        int opt = 1;
        if (setsockopt(this->listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Failed to set socket options" << std::endl;
            break;
        }
        if (bind(this->listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "Failed to bind socket" << std::endl;
            break;
        }
        if (listen(this->listen_fd, this->max_clients) < 0) {
            std::cerr << "Failed to listen on socket" << std::endl;
            break;
        }
        event* listen_event = new event(this->listen_fd, EPOLLIN | EPOLLET, this->event_buf_size);
        if (!listen_event) {
            std::cerr << "Failed to create listen event" << std::endl;
            break;
        }
        auto func = std::bind(root_connection, listen_event);
        listen_event->set(func);
        add_event(listen_event);
        // print success message
        std::cout << "Listening on " << this->ip << ":" << this->port << std::endl;
        return;
    } while (0);
    close(this->listen_fd);
    throw std::runtime_error("reactor::listen_init: Failed to create listening socket - " + std::string(strerror(errno)));
}

int reactor::wait() {
again:
    int ret =  epoll_wait(this->epoll_fd, this->epoll_events, this->max_events, this->epoll_timeout);
    if (ret < 0) {
        if (errno == EINTR) {
            goto again; // Interrupted, retry
        }
        throw std::runtime_error("Epoll wait failed:  - " + std::string(strerror(errno)));
    }
    else {
        return ret;
    }
}

bool reactor::add_event(event* ev) {
    if (ev == nullptr) {
        std::cerr << "Invalid event pointer" << std::endl;
        return false;
    }
    if (this->events.size() < this->max_events + 1) {
        this->events[ev->fd] = ev;
        ev->apply_to_reactor(this);
        return true;
    }
    return false; // 没空了
}

bool reactor::remove_event(event* ev) {
    if (ev == nullptr) {
        std::cerr << "Invalid event pointer" << std::endl;
        return false;
    }
    auto it = this->events.find(ev->fd);
    if (it != this->events.end()) {
        if (it->second->on_tree) {
            it->second->remove_from_tree();
        }
        this->events.erase(it);
        return true;
    }
    return false;
}

void event::send_message(const std::string& msg) {
    strcpy(this->buf, msg.c_str());
    size_t datasize = this->buflen = msg.size();
    int n;
    do {
        n = write_size_to(this, &datasize);
        if (n < 0) {
            std::cerr << "Failed to write response size to event" << std::endl;
            return;
        }
    } while (n == 0);
    do {
        n = write_to(this);
        if (n < 0) {
            std::cerr << "Failed to write response to event" << std::endl;
            return;
        }
        datasize -= n;
    } while (datasize > 0 || n == 0);
}

void event::recv_message(std::string& msg) {
    size_t datasize = 0;
    ssize_t n;
    do {
        n = read_size_from(this, &datasize);
        if (n < 0) {
            std::cerr << "Failed to read data size from event" << std::endl;
            return;
        }
    } while (n == 0);
    this->buflen = datasize;
    do {
        n = read_from(this);
        if (n < 0) {
            std::cerr << "Failed to read data from event" << std::endl;
            return;
        }
        datasize -= n;
    } while (datasize > 0 || n == 0);
    this->buf[this->buflen] = '\0';
    msg.assign(this->buf, this->buflen);
}