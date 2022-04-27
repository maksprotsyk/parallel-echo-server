//
// Created by mprotsyk on 27.04.22.
//

#ifndef SERVER_EVENT_H
#define SERVER_EVENT_H

#include <functional>
#include <sys/epoll.h>
#include <shared_mutex>
#include <condition_variable>

#include "abstract_event.h"
#include "abstract_async_handler.h"

class Event: public AbstractEvent {
public:
    Event(AbstractAsyncHandler* handler, int fd, Type type);
    Event(const Event& event) = delete;

    bool schedule() override;
    bool cancel() override;
    bool makeReady() override;
    bool detach() override;
    bool wait() override;
    AbstractEvent::State getState() const override;
    virtual ~Event();

protected:
    mutable std::mutex mutex;
    std::condition_variable cv;

    bool myCancel();

};



#endif //SERVER_EVENT_H
