cmake_minimum_required(VERSION 3.15)
project(server)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)


add_executable(server
        src/server.cpp
        inc/epoll_async_handler.h
        src/epoll_async_handler.cpp
        inc/poll_async_handler.h
        src/poll_async_handler.cpp
        inc/tcp_socket.h
        src/tcp_socket.cpp
        inc/abstract_async_handler.h
        inc/event.h
        src/event.cpp
        inc/io_object.h
        src/io_object.cpp
        inc/operations.h
        src/operations.cpp
        inc/echo_server.h src/echo_server.cpp src/deadline.cpp)

add_executable(client
        src/client.cpp
        inc/epoll_async_handler.h
        src/epoll_async_handler.cpp
        inc/tcp_socket.h
        src/tcp_socket.cpp
        inc/abstract_async_handler.h
        inc/event.h
        src/event.cpp
        inc/io_object.h
        src/io_object.cpp
        inc/operations.h
        src/operations.cpp
        inc/echo_client.h
        src/echo_client.cpp
        inc/deadline.h
        inc/event_data.h
        src/deadline.cpp
        )

target_include_directories(server PUBLIC inc)

target_link_libraries(server Threads::Threads)

target_link_libraries(server spdlog::spdlog)


target_include_directories(client PUBLIC inc)

target_link_libraries(client Threads::Threads)

target_link_libraries(client spdlog::spdlog)


