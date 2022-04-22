cmake_minimum_required(VERSION 3.15)
project(server)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)


add_executable(server
        src/main.cpp)

target_include_directories(server PUBLIC inc)

target_link_libraries(server Threads::Threads)

target_link_libraries(server spdlog::spdlog)


