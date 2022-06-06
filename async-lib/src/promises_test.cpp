//
// Created by maksprotsyk on 06.06.22.
//
#include <iostream>
#include <thread>


#include "poll_async_handler.h"
#include "select_async_handler.h"
#include "epoll_async_handler.h"

#include "promise.h"

using namespace std::chrono_literals;

void runHandler(EpollAsyncHandler* handler) {
    handler->runEventLoop();
}

void timer(std::function<void(std::string&&)> accept, std::function<void(int)> reject) {
    std::this_thread::sleep_for(3s);
    accept("ready");
}

void timer2(std::function<void(std::string&&)> accept, std::function<void(int)> reject) {
    reject(-1);
}

int main(int argc, char *argv[]) {
    std::cout << "RUNNING" << std::endl;
    auto* handler = new EpollAsyncHandler(100);
    std::thread thread(runHandler, handler);

    std::function asyncFunc = [](std::function<void(std::string&&)> accept, std::function<void(int)> reject) {
        std::thread thread2(timer, accept, reject);
        thread2.detach();
    };

    std::function asyncFunc2 = [](std::function<void(std::string&&)> accept, std::function<void(int)> reject) {
        std::thread thread2(timer2, accept, reject);
        thread2.detach();
    };


    Promise<std::string> promise(handler, asyncFunc);

    std::function acceptor = [](std::string x){std::cout << x + "!" << std::endl; return x + "!"; };
    std::function rejector = [](int error){return error;};

    promise.then(acceptor, rejector).then(acceptor, rejector);

    Promise<std::string> promise2(handler, asyncFunc2);
    std::function rejector2 = [](int error){std::cout << "REJECT HAPPENED" << std::endl; return error;};
    promise2.then(acceptor).then(acceptor).catch_error(rejector2);


    handler->finish();
    thread.join();


    delete handler;

    std::cout << "Finished" << std::endl;




    return 0;
}