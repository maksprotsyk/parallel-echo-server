//
// Created by mprotsyk on 28.04.22.
//

#ifndef SERVER_POLL_ASYNC_HANDLER_H
#define SERVER_POLL_ASYNC_HANDLER_H


#include <atomic>
#include <cstdlib>
#include <poll.h>
#include <unistd.h>


#include "abstract_async_handler.h"
#include "event.h"


class PollAsyncHandler: public AbstractAsyncHandler {
public:
    explicit PollAsyncHandler(size_t maxEvents);
    PollAsyncHandler(const PollAsyncHandler& handler) = delete;

    bool addEvent(Event* event) override;
    bool removeEvent(const Event* event) override;
    bool detachEvent(const Event* event) override;
    void runEventLoop() override;
    void finish() override;
private:
    size_t maxEvents;
    std::atomic<size_t> eventsNum;
    std::atomic<bool> isFinished;

    struct PollEventData {
        std::function<void()> callback;
        std::function<bool()> onProcessed;
        std::function<bool()> onReady;
        pollfd event;
    };

    std::unordered_map<int, PollEventData> eventsData;
    std::mutex mapMutex;

    static short getMode(Event::Type type);
    bool removeEvent(int eventFd);


};


#endif //SERVER_POLL_ASYNC_HANDLER_H
