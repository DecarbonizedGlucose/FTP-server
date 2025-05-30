cmake_minimum_required(VERSION 3.10)

project(FTP CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

include_directories(include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(FTP-server src/server.cpp src/reactor.cpp src/action.cpp src/serveract.cpp)

add_executable(FTP-client src/client.cpp src/action.cpp src/clientact.cpp)

install(TARGETS FTP-server DESTINATION ${CMAKE_SOURCE_DIR}/bin)
install(TARGETS FTP-client DESTINATION ${CMAKE_SOURCE_DIR}/bin)
