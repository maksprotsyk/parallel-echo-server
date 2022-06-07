//
// Created by maksprotsyk on 05.06.22.
//
#include <iostream>
#include <thread>

#include <sys/eventfd.h>
#include <sys/timerfd.h>

#include "select_async_handler.h"
#include "epoll_async_handler.h"
#include "timer_event.h"

using namespace std::chrono_literals;

void runHandler(SelectAsyncHandler *handler) {
    handler->runEventLoop();
}

void testTimedEvent() {
    auto *handler = new SelectAsyncHandler();
    std::thread thread(runHandler, handler);

    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    TimerEvent timed_event(handler, fd, 2s, Event::Type::READY_IN);

    timed_event.setCallback([]() { std::cout << "SUCCESS" << std::endl; });
    timed_event.setTimeoutCallback([]() { std::cout << "TIMEOUT" << std::endl; });
    timed_event.schedule();

    std::this_thread::sleep_for(2200ms);

    uint64_t res;
    int ret = read(fd, &res, sizeof(res));
    if (ret != -1) {
        printf("timer expired: ret = %d, exp. times = %lu\n", ret, res);
    }

    handler->finish();
    thread.join();

    delete handler;

    std::cout << "Finished" << std::endl;
}

void testEvent() {
    auto *handler = new SelectAsyncHandler();
    std::thread thread(runHandler, handler);

    int fd = eventfd(0, EFD_NONBLOCK);
    Event timed_event(handler, fd, Event::Type::READY_IN);
    timed_event.setCallback([]() { std::cout << "SUCCESS" << std::endl; });
    timed_event.setTimeoutCallback([]() { std::cout << "TIMEOUT" << std::endl; });

    timed_event.schedule(900ms);

    std::this_thread::sleep_for(1000ms);

    uint64_t x = 1;
    write(fd, &x, sizeof(uint64_t));
    handler->finish();
    thread.join();

    delete handler;

    std::cout << "Finished" << std::endl;

}

int main(int argc, char *argv[]) {
    testTimedEvent();
    testEvent();
    return 0;
}