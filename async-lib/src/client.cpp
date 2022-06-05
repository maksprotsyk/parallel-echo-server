#include <iostream>
#include <thread>

#include "epoll_async_handler.h"
#include "select_async_handler.h"
#include "tcp_socket.h"
#include "operations.h"
#include "io_object.h"

#include "echo_client.h"

void runHandler(EpollAsyncHandler* handler) {
    handler->runEventLoop();
}


int main(int argc, char *argv[]) {
    auto logger = spdlog::stdout_color_mt(LOGGER_NAME);

    logger->set_pattern("%^[%d-%m-%Y %H:%M:%S.%e] [%l] [%s:%#] [%!] [thread %t]%$ %v");
    logger->set_level(spdlog::level::level_enum::debug);
    //spdlog::register_logger(logger);
    spdlog::flush_every(std::chrono::seconds(3));

    auto* handler = new EpollAsyncHandler(100);
    std::thread thread(runHandler, handler);

    EchoClient client(handler, "127.0.0.1", 8080);
    client.run();


    thread.join();

    delete handler;



   return 0;
}