#ifndef SERVERACT_HPP
#define SERVERACT_HPP

#include "../include/action.hpp"
#include "../include/thread_pool.hpp"
#include "../include/reactor.hpp"
#include <iostream>
#include <stdexcept>

void init_root_dir(const char* str);

// for event[flag=0]
// create event[flag=1]
void root_connection(event* ev);

/* for event[flags=1]
 * commands:
 * 1. PASV: 创建数据传输通道
 * 2. STOR <file>: 客户端上传文件(服务端下载)
 * 3. RETR <file>: 客户端下载文件(服务端上传)
 * 4. ls: 列出当前目录下的文件
 * 5. cd <dir>: 切换目录
 * 6. mkdir <dir>: 创建目录
 * 7. rmdir <dir>: 删除目录
 * 8. rm <file>: 删除文件
 * 9. dic: 断开通道
*/
void command_analyser(event* ev);

// for event[flag=1]
// create event[flag=2]
void new_data_channel_listen(event* ev);

// for event[flag=1]
void notify_client(event* ev);

// for event[flag=2]
// create event[flag=3]
void create_data_channel(event* ev);

// for event[flag=1]
void download(event* ev, std::string arg);

// for event[flag=1]
void upload(event* ev, std::string arg);

// for event[flag=1]
void list_dir(event* ev);

// for event[flag=1]
void change_dir(event* ev, std::string arg);

// for event[flag=1]
void create_dir(event* ev, std::string arg);

// for event[flag=1]
void remove_dir(event* ev, std::string arg);

// for event[flag=1]
void remove_file(event* ev, std::string arg);

// for event[flag=1]
// destroy event[flag=2 flag=3]
void close_channel(event* ev, bool flag);

// for event[flag=1]
// destroy ev self (ev[flag=1])
void disconnect(event* ev);

// for event[flag=1]
// 中途发送响应或者提示
void send_resp(event* ev, const std::string& resp);
void send_resp_again(event* ev, const std::string& resp1, const std::string& resp2);

#endif