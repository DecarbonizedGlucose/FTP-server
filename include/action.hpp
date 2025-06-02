#ifndef ACTION_HPP
#define ACTION_HPP

#include <string>
#include <stdexcept>
#include <iostream>
#include <functional>
#include "../include/reactor.hpp"

/*
 * 这个文件包含关于通信的行为以及本地的行为
*/

/* ---------- io funcs ---------- */

int read_size_from(event* e, int* datasize);
int read_size_from(int fd, int* datasize);

int write_size_to(event* e, int* datasize);
int write_size_to(int fd, int* datasize);

int read_from(event* e);
int read_from(int fd, char* buf, int buf_size, int* buflen);

int write_to(event* e);
int write_to(int fd, const char* buf, int buf_size, int* buflen);

/* ---------- file system ---------- */

class file_manager {
private:
    std::string root_dir;
    std::string current_dir;
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

};

#endif