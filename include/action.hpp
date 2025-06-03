#ifndef ACTION_HPP
#define ACTION_HPP

#include <string>
#include <stdexcept>
#include <iostream>
#include <functional>
#include <unistd.h>
#include "../include/reactor.hpp"

/*
 * 这个文件包含关于通信的行为以及本地的行为
*/

/* ---------- io funcs ---------- */

ssize_t read_size_from(event* e, size_t* datasize);
ssize_t read_size_from(int fd, size_t* datasize);

ssize_t write_size_to(event* e, size_t* datasize);
ssize_t write_size_to(int fd, size_t* datasize);

ssize_t read_from(event* e);
ssize_t read_from(int fd, char* buf, size_t buf_size, size_t* buflen);

ssize_t write_to(event* e);
ssize_t write_to(int fd, const char* buf, size_t buf_size, size_t* buflen);

/* ---------- file system ---------- */

class file_manager {
private:
    std::string root_dir;
    std::string current_dir;

    std::string get_real_path(const std::string path, bool ex = true);

    ssize_t copy_file(
        int from_fd, int to_fd, char* buf,
        size_t buf_size, size_t* buflen, size_t file_size);
public:
    file_manager() = delete;
    file_manager(std::string root_dir);
    ~file_manager() = default;

    std::string get_dir_display();
    std::string get_dir_display(const std::string& dir);

    bool ls(std::string& resp);
    bool cd(const std::string& dir, std::string& resp);
    bool mkdir(const std::string& new_dir, std::string& resp);
    bool rmdir(const std::string& dir_to_del, std::string& resp);
    bool rm(const std::string& file_to_del, std::string& resp);

    ssize_t upload(
        const std::string& file_to_upload,
        int sockfd,
        char* buf,
        size_t bufsize,
        size_t* buflen,
        std::string& resp,
        char flag);

    ssize_t download(
        const std::string& file_to_download,
        int fd,
        char* buf,
        size_t bufsize,
        size_t* buflen,
        std::string& resp,
        char flag);
};

#endif