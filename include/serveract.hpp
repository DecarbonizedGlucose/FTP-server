#ifndef SERVERACT_HPP
#define SERVERACT_HPP

#include "../include/action.hpp"
#include "../include/thread_pool.hpp"
#include "../include/reactor.hpp"
#include <iostream>
#include <stdexcept>

// flag = 0
void root_connection(event* ev);

// flag = 1
void command_analyser(event* ev);

// flag = 1
void new_data_channel_listen(event* ev);

// flag = 1
void notify_client(event* ev);

// flag = 2
void create_data_channel(event* ev);

//flag = 3
void download(event* ev);

// flag = 3
void upload(event* ev);

// flag = 1
void list_dir(event* ev);

// flag = 1
void change_dir(event* ev, std::string arg);

// flag = 1
void create_dir(event* ev, std::string arg);

// flag = 1
void remove_dir(event* ev, std::string arg);

// flag = 1
void remove_file(event* ev, std::string arg);

// flag = 1
void disconnect(event* ev);

#endif