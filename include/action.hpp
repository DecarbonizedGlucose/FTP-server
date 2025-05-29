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

void recv_data(event* ev);

void send_data(event* ev);

void test_recv_data(event* e);

void test_send_data(event* e);

int read_size_from(event* e, int* datasize);
int read_size_from(int fd, int* datasize);

int write_size_to(event* e, int* datasize);
int write_size_to(int fd, int* datasize);

int read_from(event* e);
int read_from(int fd, char* buf, int buf_size, int* buflen);

int write_to(event* e);
int write_to(int fd, const char* buf, int buf_size, int* buflen);

#endif