//
// Created by mprotsyk on 28.04.22.
//

#ifndef SERVER_POLL_ASYNC_HANDLER_H
#define SERVER_POLL_ASYNC_HANDLER_H


#include <atomic>
#include <cstdlib>
#include <poll.h>
#include <unistd.h>
#include <queue>

#include "abstract_async_handler.h"
#include "event.h"
#include "deadline.h"
#include "event_data.h"


class PollAsyncHandler: public AbstractAsyncHandler {
public:
    explicit PollAsyncHandler(size_t maxEvents);
    PollAsyncHandler(const PollAsyncHandler& handler) = delete;
    virtual ~PollAsyncHandler();
    void runEventLoop() override;
    void finish() override;

protected:
    bool addEvent(Event& event) override;
    bool addEvent(Event& event, const std::chrono::milliseconds& ms) override;
    bool removeEvent(const Event& event) override;
    bool detachEvent(const Event& event) override;

private:
    size_t maxEvents;
    std::atomic<size_t> eventsNum;
    std::atomic<bool> isFinished;

    std::priority_queue<Deadline> deadlines;
    std::unordered_map<int, std::pair<EventData, size_t>> data;
    std::mutex queueMutex;
    std::mutex pollMutex;
    std::mutex mapMutex;

    pollfd* eventDescriptors;


    static short getMode(Event::Type type);
    bool removeEvent(int eventFd);



};


#endif //SERVER_POLL_ASYNC_HANDLER_H
