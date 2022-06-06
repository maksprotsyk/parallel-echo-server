//
// Created by mprotsyk on 27.04.22.
//

#ifndef SERVER_EPOLL_ASYNC_HANDLER_H
#define SERVER_EPOLL_ASYNC_HANDLER_H

#include <atomic>
#include <cstdlib>
#include <sys/epoll.h>
#include <unistd.h>
#include <queue>

#include "abstract_async_handler.h"
#include "event.h"
#include "deadline.h"
#include "event_data.h"


class EpollAsyncHandler: public AbstractAsyncHandler {
public:
    explicit EpollAsyncHandler(size_t maxEvents);
    EpollAsyncHandler(const EpollAsyncHandler& handler) = delete;

    void runEventLoop() override;
    void finish() override;
    ~EpollAsyncHandler();
protected:
    bool addEvent(Event& event) override;
    bool addEvent(Event& event, const std::chrono::milliseconds& ms) override;
    bool removeEvent(const Event& event) override;
    bool detachEvent(const Event& event) override;
private:
    size_t maxEvents;
    epoll_event* events;
    std::atomic<size_t> eventsNum;
    int epollFd;
    std::atomic<bool> isFinished;

    std::priority_queue<Deadline> deadlines;
    std::unordered_map<int, EventData> data;
    std::mutex queueMutex;
    std::mutex epollMutex;
    std::mutex mapMutex;

    static int getMode(Event::Type type);
    bool removeEvent(int eventFd);


};


#endif //SERVER_EPOLL_ASYNC_HANDLER_H
