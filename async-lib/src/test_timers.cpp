//
// Created by maksprotsyk on 05.06.22.
//
#include <iostream>
#include <thread>

#include <sys/eventfd.h>

#include "poll_async_handler.h"
#include "select_async_handler.h"
#include "epoll_async_handler.h"

using namespace std::chrono_literals;

void runHandler(SelectAsyncHandler* handler) {
    handler->runEventLoop();
}


int main(int argc, char *argv[]) {
    auto* handler = new SelectAsyncHandler();
    std::thread thread(runHandler, handler);


    int fd = eventfd(0, EFD_NONBLOCK);
    Event timed_event(handler, fd, Event::Type::READY_IN);
    timed_event.setCallback([](){std::cout << "SUCCESS" << std::endl;});
    timed_event.setTimeoutCallback([](){std::cout << "TIMEOUT" << std::endl;});

    timed_event.schedule(900ms);

    std::this_thread::sleep_for(1000ms);

    uint64_t x = 1;
    write(fd, &x, sizeof(uint64_t));

    handler->finish();
    thread.join();


    delete handler;

    std::cout << "Finished" << std::endl;



    return 0;
}