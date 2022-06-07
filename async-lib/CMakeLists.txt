cmake_minimum_required(VERSION 3.15)
project(async-lib)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)

add_library(async
        src/epoll_async_handler.cpp
        src/poll_async_handler.cpp
        src/tcp_socket.cpp
        src/event.cpp
        inc/io_object.h
        src/io_object.cpp
        src/operations.cpp
        src/echo_server.cpp
        src/echo_client.cpp
        src/deadline.cpp
        src/select_async_handler.cpp
        src/timer_event.cpp inc/timer_event.h)


target_include_directories(async PUBLIC inc)

target_link_libraries(async Threads::Threads)

target_link_libraries(async spdlog::spdlog)


add_executable(server
        src/server.cpp
        )

target_link_libraries(server async)


add_executable(client
        src/client.cpp
        )

target_link_libraries(client async)


add_executable(test_timers
        src/test_timers.cpp)

target_link_libraries(test_timers async)




