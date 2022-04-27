cmake_minimum_required(VERSION 3.15)
project(server)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)


add_executable(server
        src/main.cpp
        inc/event.h
        inc/epoll_async_handler.h
        src/event.cpp
        src/epoll_async_handler.cpp
        inc/socket_server.h
        src/socket_server.cpp
        inc/abstract_async_handler.h
        inc/abstract_event.h
        src/abstract_event.cpp
        inc/io_object.h
        src/io_object.cpp
        inc/operations.h
        src/operations.cpp
        inc/echo_server.h src/echo_server.cpp)

target_include_directories(server PUBLIC inc)

target_link_libraries(server Threads::Threads)

target_link_libraries(server spdlog::spdlog)


