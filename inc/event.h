//
// Created by mprotsyk on 27.04.22.
//

#ifndef SERVER_EVENT_H
#define SERVER_EVENT_H

#include <functional>
#include <mutex>
#include <condition_variable>

#include "abstract_async_handler.h"

class AbstractAsyncHandler;

class Event {
public:
    enum class State {
        INIT, SCHEDULED, READY, CANCELED, DETACHED, PROCESSED
    };

    enum class Type {
        READY_IN, READY_OUT, DISCONNECTION
    };


    Event(AbstractAsyncHandler* handler_, int fd_, Type type_);
    Event(const Event& event) = delete;

    bool schedule();
    bool cancel();
    bool makeReady();
    bool makeProcessed();
    bool detach();
    bool wait();
    State getState() const;
    ~Event();

    void setCallback(const std::function<void()>& callback);
    [[nodiscard]] std::function<void()> getCallback() const ;
    [[nodiscard]] Type getType() const;
    [[nodiscard]] int getDescriptor() const;

protected:
    int fd;
    Type type;
    AbstractAsyncHandler* handler;
    std::function<void()> callback;
    State state;
    mutable std::mutex mutex;
    std::condition_variable cv;

    bool myCancel();

};

#endif //SERVER_EVENT_H
