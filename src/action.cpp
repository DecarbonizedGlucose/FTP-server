#include "../include/action.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h> // for PATH_MAX
#include <stdlib.h> // for realpath
using fm = file_manager;

/* ---------- io funcs ---------- */

ssize_t read_size_from(event* e, size_t* datasize) {
    if (!e || !e->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return -1;
    }
    ssize_t n;
again:
    n = read(e->fd, datasize, sizeof(size_t));
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
    else if (n != sizeof(size_t)) {
        std::cerr << "Read size mismatch: expected " << sizeof(size_t) << ", got " << n << std::endl;
        return 0;
    }
    return n;
}

ssize_t read_size_from(int fd, size_t* datasize) {
    if (fd < 0 || !datasize) {
        std::cerr << "Invalid file descriptor or data size pointer" << std::endl;
        return false;
    }
    ssize_t n;
again:
    n = read(fd, datasize, sizeof(size_t));
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
    else if (n != sizeof(size_t)) {
        std::cerr << "Read size mismatch: expected " << sizeof(size_t) << ", got " << n << std::endl;
        return 0;
    }
    return n;
}

ssize_t write_size_to(event* e, size_t* datasize) {
    if (!e || !e->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return 0;
    }
    ssize_t n;
again:
    n = write(e->fd, datasize, sizeof(size_t));
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
    else if (n != sizeof(size_t)) {
        std::cerr << "Write size mismatch: expected " << sizeof(size_t) << ", got " << n << std::endl;
        return -1;
    }
    return n;
}

ssize_t write_size_to(int fd, size_t* datasize) {
    ssize_t n;
again:
    n = write(fd, datasize, sizeof(size_t));
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
    else if (n != sizeof(size_t)) {
        std::cerr << "Write size mismatch: expected " << sizeof(size_t) << ", got " << n << std::endl;
        return -1;
    }
    return n;
}

ssize_t read_from(event* e) {
    if (!e || !e->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return -1;
    }
    ssize_t n;
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

ssize_t read_from(int fd, char* buf, size_t buf_size, size_t* buflen) {
    if (fd < 0 || !buf || buf_size <= 0) {
        std::cerr << "Invalid file descriptor or buffer" << std::endl;
        return -1;
    }
    ssize_t n;
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

ssize_t write_to(event* e) {
    if (!e || !e->p_rea) {
        std::cerr << "Invalid event or reactor pointer" << std::endl;
        return -1;
    }
    ssize_t n;
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

ssize_t write_to(int fd, const char* buf, size_t buf_size, size_t* buflen) {
    if (fd < 0 || !buf || buf_size <= 0) {
        std::cerr << "Invalid file descriptor or buffer" << std::endl;
        return -1;
    }
    ssize_t n;
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
            if (entry->d_type == DT_DIR) {
                resp += '/'; // Append '/' for directories
            }
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
    std::string full_path;
    if (dir[0] == '/') {
        full_path = root_dir + dir.substr(1); // Absolute path
    }
    else {
        full_path = current_dir + dir; // Relative path
    }
    full_path = get_real_path(full_path);
    if (full_path.back() != '/') {
        full_path += '/';
    }
    // 必须以 root_dir 为前缀
    if (full_path.find(root_dir) != 0) {
        resp = "Error: Access denied\n";
        return false;
    }
    DIR* dir_ptr = opendir(full_path.c_str());
    if (!dir_ptr) {
        std::cerr << "Failed to change directory " << full_path << ": " << strerror(errno) << std::endl;
        resp = "Error: Unable to change directory\n";
        return false;
    }
    closedir(dir_ptr);
    current_dir = full_path;
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
    // 规范化路径
    full_path = get_real_path(full_path, false);
    if (full_path.back() != '/') {
        full_path += '/';
    }
    if (full_path.find(root_dir) == std::string::npos) {
        resp = "Error: Access denied\n";
        return false;
    }
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
    // 规范化路径
    full_path = get_real_path(full_path);
        if (full_path.back() != '/') {
        full_path += '/';
    }
    if (full_path.find(root_dir) == std::string::npos || full_path.find(current_dir) == 0){
        resp = "Error: Only sub directories removable\n";
        return false;
    }
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
    if (file_to_del.empty() || file_to_del == "/") {
        resp = "Error: Invalid file name\n";
        return false;
    }
    std::string full_path;
    if (file_to_del[0] == '/') {
        full_path = root_dir + file_to_del.substr(1);
    }
    else {
        full_path = current_dir + file_to_del;
    }
    full_path = get_real_path(full_path);
    if (full_path.find(root_dir) != 0) {
        resp = "Error: Only sub files removable\n";
        return false;
    }
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

ssize_t fm::copy_file(
    int from_fd, int to_fd, char* buf,
    size_t buf_size, size_t* buflen, size_t file_size) {
    if (from_fd < 0 || to_fd < 0 || buf == nullptr || buf_size <= 0 || buflen == nullptr || file_size < -1) {
        std::cerr << "Invalid parameters for copy_file" << std::endl;
        return -1;
    }
    // 这里要判断file_size的值。如果是-1, 表示要先读来size, 是download操作
    // 如果size >= 0, 要先写入size, 是upload操作
    ssize_t n;
    size_t size_;
    if (file_size == -1) {
        do {
            n = read_size_from(from_fd, &size_);
        } while (n == 0);
        if (n < 0) {
            std::cerr << "Failed to read file size from source" << std::endl;
            return -1;
        }
        file_size = size_;
    }
    else {
        size_ = file_size;
        do {
            n = write_size_to(to_fd, &size_);
        } while (n == 0);
        if (n < 0) {
            std::cerr << "Failed to write file size to destination" << std::endl;
            return -1;
        }
    }
    // 由from_fd读取数据到buf, 然后写入to_fd
    size_t total_read = 0, total_written = 0;
    while (total_written < file_size) {
        *buflen = 0;
        do {
            n = read_from(from_fd, buf, buf_size, buflen);
        } while (n == 0);
        if (n < 0) {
            std::cerr << "Failed to read data from source" << std::endl;
            return -1;
        }
        else if (n == 0) {
            std::cerr << "Source file ended unexpectedly" << std::endl;
            return total_written;
        }
        total_read += n;
        do {
            n = write_to(to_fd, buf, buf_size, buflen);
        } while (n == 0);
        if (n < 0) {
            std::cerr << "Failed to write data to destination" << std::endl;
            return -1;
        }
        else if (n == 0) {
            std::cerr << "Destination file ended unexpectedly" << std::endl;
            return total_written;
        }
        total_written += n;
    }
    if (total_written != file_size) {
        std::cerr << "File size mismatch: expected " << file_size << ", got " << total_written << std::endl;
        return -1;
    }
    return total_written;
}

ssize_t fm::upload(
    const std::string& file_to_upload,
    int sockfd,
    char* buf,
    size_t buf_size,
    size_t* buflen,
    std::string& resp,
    char flag
) {
    if (file_to_upload.empty()) {
        resp = "Error: File name cannot be empty\n";
        return -1;
    }
    std::string full_path;
    if (flag == 's')
        full_path = get_real_path(current_dir + file_to_upload);
    else
        full_path = get_real_path(file_to_upload);
    if (full_path.empty()) {
        resp = "Error: Invalid file path\n";
        return -1;
    }
    int file_fd = open(full_path.c_str(), O_RDONLY);
    if (file_fd < 0) {
        resp = "Error: Unable to open file " + full_path + ": " + strerror(errno) + "\n";
        return -1;
    }
    // 获取文件大小
    struct stat file_stat;
    if (fstat(file_fd, &file_stat) < 0) {
        close(file_fd);
        resp = "Error: Unable to get file size for " + full_path + ": " + strerror(errno) + "\n";
        return -1;
    }
    off_t file_size = file_stat.st_size;
    ssize_t ret = copy_file(
        file_fd, sockfd,
        buf, buf_size, buflen, file_size);
    close(file_fd);
    if (ret < 0) {
        if (flag == 'c')
            resp = "Error: Client: File upload failed\n";
        else
            resp = "Error: Server: File download failed\n";
        return -1;
    }
    if (flag == 'c') {
        resp = "Client: Successfully uploaded file: " + get_dir_display(full_path) + "\n";
    } else {
        resp = "Server: Successfully downloaded file: " + get_dir_display(full_path) + "\n";
    }
    resp += "File size: " + std::to_string(file_size) + " bytes\n";
    return ret;
}

ssize_t fm::download(
    const std::string& file_to_download,
    int fd,
    char* buf,
    size_t bufsize,
    size_t* buflen,
    std::string& resp,
    char flag
) {
    if (file_to_download.empty()) {
        resp = "Error: File name cannot be empty\n";
        return -1;
    }
    std::string full_path;
    if (flag == 's')
        full_path = get_real_path(current_dir + file_to_download, false);
    else
        full_path = get_real_path(file_to_download, false);
    if (full_path.empty()) {
        resp = "Error: Invalid file path\n";
        return -1;
    }
    // 根据flag判断是客户端还是服务器端
    if (flag == 's') {
        if (full_path.find(root_dir) != 0) {
            resp = "Error: Only sub dirs able to write\n";
            return -1;
        }
    }
    int file_fd = open(full_path.c_str(), O_RDWR | O_TRUNC);
    if (file_fd < 0) {
        resp = "Error: Unable to open file " + full_path + ": " + strerror(errno) + "\n";
        return -1;
    }
    off_t file_size = copy_file(
        fd, file_fd,
        buf, bufsize, buflen, -1);
    close(file_fd);
    if (file_size < 0) {
        if (flag == 'c')
            resp = "Error: Client: File download failed\n";
        else
            resp = "Error: Server: File upload failed\n";
        return -1;
    }
    if (flag == 'c') {
        resp = "Client: Successfully downloaded file: " + get_dir_display(full_path) + "\n";
    } else {
        resp = "Server: Successfully uploaded file: " + get_dir_display(full_path) + "\n";
    }
    resp += "File size: " + std::to_string(file_size) + " bytes\n";
    return file_size;
}

std::string fm::get_real_path (const std::string path, bool ex) {
    char resolved[PATH_MAX];
    if (!realpath(path.c_str(), resolved)) {
        if (ex || errno != ENOENT) {
            return "";
        }
    }
    return std::string(resolved);
}

