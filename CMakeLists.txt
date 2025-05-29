cmake_minimum_required(VERSION 3.10)

project(FTP CXX)
set(CMAKE_CXX_STANDARD 11)

include_directories(include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

add_executable(FTP-server src/server.cpp src/reactor.cpp src/action.cpp src/serveract.cpp)

add_executable(FTP-client src/client.cpp src/action.cpp src/clientact.cpp)

install(TARGETS FTP-server DESTINATION bin)
install(TARGETS FTP-client DESTINATION bin)
