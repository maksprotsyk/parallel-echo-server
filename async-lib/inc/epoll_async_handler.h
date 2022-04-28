//
// Created by mprotsyk on 27.04.22.
//

#ifndef SERVER_EPOLL_ASYNC_HANDLER_H
#define SERVER_EPOLL_ASYNC_HANDLER_H

#include <atomic>
#include <cstdlib>
#include <sys/epoll.h>
#include <unistd.h>


#include "abstract_async_handler.h"
#include "event.h"


class EpollAsyncHandler: public AbstractAsyncHandler {
public:
    explicit EpollAsyncHandler(size_t maxEvents);
    EpollAsyncHandler(const EpollAsyncHandler& handler) = delete;

    bool addEvent(Event* event) override;
    bool removeEvent(const Event* event) override;
    bool detachEvent(const Event* event) override;
    void runEventLoop() override;
    void finish() override;
    ~EpollAsyncHandler();
private:
    size_t maxEvents;
    epoll_event* events;
    std::atomic<size_t> eventsNum;
    int fd;
    std::atomic<bool> isFinished;

    static int getMode(Event::Type type);
    bool removeEvent(int eventFd);


};


#endif //SERVER_EPOLL_ASYNC_HANDLER_H