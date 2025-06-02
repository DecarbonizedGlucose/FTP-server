#include "../include/action.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h> // for PATH_MAX
#include <stdlib.h> // for realpath
using fm = file_manager;

/* ---------- io funcs ---------- */

int read_size_from(event* e, int* datasize) {
    if (!e || !e->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return -1;
    }
    int n;
again:
    n = read(e->fd, datasize, sizeof(int));
    if (n == -1) {
        if (errno == EINTR) {
            goto again;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            //std::cerr << "No data available to read from client: " << e->fd << std::endl;
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
    else if (n != sizeof(int)) {
        std::cerr << "Read size mismatch: expected " << sizeof(int) << ", got " << n << std::endl;
        return 0;
    }
    return n;
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
            //std::cerr << "No data available to read from fd: " << fd << std::endl;
            return 0;
        }
        else {
            std::cerr << "Read error: " << strerror(errno) << std::endl;
            return -1;
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
    return n;
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
        else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            //std::cerr << "No space available to write to client: " << e->fd << std::endl;
            return 0;
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
    return n;
}

int write_size_to(int fd, int* datasize) {
    int n;
again:
    n = write(fd, datasize, sizeof(int));
    if (n == -1) {
        if (errno == EINTR) {
            goto again;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            //std::cerr << "No space available to write to fd: " << fd << std::endl;
            return 0;
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
    return n;
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
            //std::cerr << "No data available to read from client: " << e->fd << std::endl;
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
            //std::cerr << "No data available to read from client: " << fd << std::endl;
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
        else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            //std::cerr << "No space available to write to client: " << e->fd << std::endl;
            return 0;
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
        else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            //std::cerr << "No space available to write to fd: " << fd << std::endl;
            return 0;
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

/* ---------- file system ---------- */

fm::file_manager(std::string root_dir) : root_dir(root_dir) {
    if (root_dir.empty()) {
        throw std::invalid_argument("Root directory cannot be empty");
    }
    if (root_dir.back() != '/') {
        root_dir += '/';
    }
    current_dir = root_dir;
}

std::string fm::get_dir_display() {
    return "(root)/" + current_dir.substr(root_dir.length());
}

std::string fm::get_dir_display(const std::string& dir) {
    return "(root)/" + dir.substr(root_dir.length());
}

bool fm::ls(std::string& resp) {
    DIR* dir = opendir(current_dir.c_str());
    if (!dir) {
        std::cerr << "Failed to open directory: " << strerror(errno) << std::endl;
        resp = "Error: Unable to list directory" + get_dir_display() + "\n";
        return false;
    }
    struct dirent* entry;
    resp.clear();
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] != '.') { // Skip hidden files
            resp += entry->d_name;
            resp += '\n';
        }
    }
    closedir(dir);
    if (resp.empty()) {
        resp = "(empty)\n";
    }
    resp = get_dir_display() + ":\n" + resp;
    return true;
}

bool fm::cd(const std::string& dir, std::string& resp) {
    if (dir.empty() || dir == "/") {
        current_dir = root_dir;
        resp = "Changed directory to root\n";
        return true;
    }
    std::string new_dir;
    if (dir[0] == '/') {
        new_dir = root_dir + dir.substr(1); // Absolute path
    }
    else {
        new_dir = current_dir + dir; // Relative path
    }
    if (new_dir.back() != '/') {
        new_dir += '/';
    }
    // 规范化路径
    char resolved[PATH_MAX];
    if (!realpath(new_dir.c_str(), resolved)) {
        resp = "Error: Invalid path\n";
        return false;
    }
    std::string abs_path(resolved);
    // 必须以 root_dir 为前缀
    if (abs_path.find(root_dir) != 0) {
        resp = "Error: Access denied\n";
        return false;
    }
    DIR* dir_ptr = opendir(abs_path.c_str());
    if (!dir_ptr) {
        std::cerr << "Failed to change directory " << abs_path << ": " << strerror(errno) << std::endl;
        resp = "Error: Unable to change directory\n";
        return false;
    }
    closedir(dir_ptr);
    current_dir = abs_path + '/';
    resp = "Changed directory to " + get_dir_display() + "\n";
    return true;
}

bool fm::mkdir(const std::string& new_dir, std::string& resp) {
    if (new_dir.empty() || new_dir == "/") {
        resp = "Error: Invalid directory name\n";
        return false;
    }
    std::string full_path;
    if (new_dir[0] == '/') {
       full_path = root_dir + new_dir.substr(1); // Absolute path
    }
    else {
        full_path = current_dir + new_dir; // Relative path
    }
    if (full_path.back() != '/') {
        full_path += '/';
    }
    // 规范化路径
    char resolved[PATH_MAX];
    if (!realpath(full_path.c_str(), resolved)) {
        resp = "Error: Invalid path\n";
        return false;
    }
    full_path = std::string(resolved);
    if (full_path.find(root_dir) != 0) {
        resp = "Error: Access denied\n";
        return false;
    }
    full_path.assign(resolved);
    int ret = ::mkdir(full_path.c_str(), 0755);
    if (ret == 0) {
        resp = "Successfully created directory: " + get_dir_display(full_path) + "\n";
        return true;
    }
    else if (errno == EEXIST) {
        resp = "Directory " + get_dir_display(full_path) + " already exists.\n";
        return false;
    }
    else {
        resp = "Error: Unable to create directory " + get_dir_display(full_path) + ": " + strerror(errno) + "\n";
        std::cerr << resp;
        return false;
    }
}

bool fm::rmdir(const std::string& dir_to_del, std::string& resp) {
    std::string full_path;
    if (dir_to_del.empty() || dir_to_del == "/") {
        resp = "Error: Invalid directory name\n";
        return false;
    }
    if (dir_to_del[0] == '/') {
        full_path = root_dir + dir_to_del.substr(1); // Absolute path
    }
    else {
        full_path = current_dir + dir_to_del; // Relative path
    }
    if (full_path.back() != '/') {
        full_path += '/';
    }
    // 规范化路径
    char resolved[PATH_MAX];
    if (!realpath(full_path.c_str(), resolved)) {
        resp = "Error: Invalid path\n";
        return false;
    }
    full_path = std::string(resolved);
    if (full_path.find(root_dir) != 0 || current_dir.find(full_path) == 0) {
        resp = "Error: Only sub derecotories removable\n";
        return false;
    }
    full_path.assign(resolved);
    int ret = ::rmdir(full_path.c_str());
    if (ret == 0) {
        resp = "Successfully removed directory: " + get_dir_display(full_path) + "\n";
        return true;
    }
    else if (errno == ENOENT) {
        resp = "Directory " + get_dir_display(full_path) + " does not exist.\n";
        return false;
    }
    else if (errno == ENOTEMPTY) {
        resp = "Directory " + get_dir_display(full_path) + " is not empty.\n";
        return false;
    }
    else {
        resp = "Error: Unable to remove directory " + get_dir_display(full_path) + ": " + strerror(errno) + "\n";
        std::cerr << resp;
        return false;
    }
}

bool fm::rm(const std::string& file_to_del, std::string& resp) {
    std::string full_path = current_dir + file_to_del;
    int ret = ::unlink(full_path.c_str());
    if (ret == 0) {
        resp = "Successfully removed file: " + get_dir_display(full_path) + "\n";
        return true;
    }
    else if (errno == ENOENT) {
        resp = "File " + get_dir_display(full_path) + " does not exist.\n";
        return false;
    }
    else {
        resp = "Error: Unable to remove file " + get_dir_display(full_path) + ": " + strerror(errno) + "\n";
        std::cerr << resp;
        return false;
    }
}

