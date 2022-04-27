//
// Created by mprotsyk on 27.04.22.
//

#ifndef SERVER_ABSTRACT_EVENT_H
#define SERVER_ABSTRACT_EVENT_H

#include <functional>

class AbstractAsyncHandler;

class AbstractEvent {
public:
    enum class State {
        INIT, SCHEDULED, READY, CANCELED, DETACHED
    };

    enum class Type {
        READY_IN, READY_OUT, DISCONNECTION
    };


    AbstractEvent(AbstractAsyncHandler* handler_, int fd_, Type type_);
    AbstractEvent(const AbstractEvent& event) = delete;

    virtual bool schedule() = 0;
    virtual bool cancel() = 0;
    virtual bool makeReady() = 0;
    virtual bool detach() = 0;
    virtual bool wait() = 0;
    [[nodiscard]] virtual State getState() const = 0;
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
};

#endif //SERVER_ABSTRACT_EVENT_H
