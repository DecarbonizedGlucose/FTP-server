#pragma once
#include <string>
#include <stdexcept>
#include <iostream>
#include <functional>
#include "../include/ftp.hpp"
#include "../include/reactor.hpp"

/*
 * 这个文件包含关于通信的行为以及本地的行为
*/

void recv_data(event* ev);

void send_data(event* ev);

void accept_connection(event* ev);

void test_recv_data(event* e);

void test_send_data(event* e);

int read_size_from(event* e, int* size);

int write_size_to(event* e, int* size);

int read_from(event* e);

int write_to(event* e);