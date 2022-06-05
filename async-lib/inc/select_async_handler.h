//
// Created by maksprotsyk on 05.06.22.
//

#ifndef SERVER_SELECT_ASYNC_HANDLER_H
#define SERVER_SELECT_ASYNC_HANDLER_H


#include <atomic>
#include <cstdlib>
#include <sys/select.h>
#include <unistd.h>
#include <queue>
#include <map>
#include <set>

#include "abstract_async_handler.h"
#include "event.h"
#include "deadline.h"
#include "event_data.h"


class SelectAsyncHandler: public AbstractAsyncHandler {
public:
    explicit SelectAsyncHandler();
    SelectAsyncHandler(const SelectAsyncHandler& handler) = delete;

    bool addEvent(Event* event) override;
    bool addEvent(Event* event, const std::chrono::milliseconds& ms) override;
    bool removeEvent(const Event* event) override;
    bool detachEvent(const Event* event) override;
    void runEventLoop() override;
    void finish() override;
private:

    std::vector<std::set<int>> sets;



    std::atomic<size_t> eventsNum;
    std::atomic<bool> isFinished;

    std::priority_queue<Deadline> deadlines;
    std::map<int, EventData> data;
    std::mutex queueMutex;
    std::mutex selectMutex;
    std::mutex mapMutex;

    bool removeEvent(int eventFd);
    static size_t getType(Event::Type type);


};


#endif //SERVER_SELECT_ASYNC_HANDLER_H
