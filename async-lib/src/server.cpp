//
// Created by mprotsyk on 28.04.22.
//
#include <iostream>
#include <thread>

#include "poll_async_handler.h"
#include "select_async_handler.h"
#include "epoll_async_handler.h"
#include "echo_server.h"
#include "logging.h"



int main(int argc, char *argv[]) {
    auto logger = spdlog::stdout_color_mt(LOGGER_NAME);

    logger->set_pattern("%^[%d-%m-%Y %H:%M:%S.%e] [%l] [%s:%#] [%!] [thread %t]%$ %v");
    logger->set_level(spdlog::level::level_enum::off);
    //spdlog::register_logger(logger);
    spdlog::flush_every(std::chrono::seconds(3));

    auto* handler = new PollAsyncHandler(1000);

    EchoServer server(handler, "127.0.0.1", 8080);

    handler->runEventLoop();

    delete handler;



    return 0;
}