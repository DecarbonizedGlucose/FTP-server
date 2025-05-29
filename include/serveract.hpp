#ifndef SERVERACT_HPP
#define SERVERACT_HPP

#include "../include/action.hpp"
#include "../include/thread_pool.hpp"
#include "../include/reactor.hpp"
#include <iostream>
#include <stdexcept>

void root_connection(event* ev);

void command_analyser(event* ev);

void new_data_channel_listen(event* ev);

void notify_client(event* ev, event* s_ev);

void create_data_channel(event* ev);

#endif