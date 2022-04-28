//
// Created by mprotsyk on 27.04.22.
//

#ifndef SERVER_ABSTRACT_ASYNC_HANDLER_H
#define SERVER_ABSTRACT_ASYNC_HANDLER_H

#include <memory>

#include "event.h"

class Event;

class AbstractAsyncHandler {
public:
    virtual bool addEvent(Event* event) = 0;
    virtual bool removeEvent(const Event* event) = 0;
    virtual bool detachEvent(const Event* event) = 0;
    virtual void runEventLoop() = 0;
    virtual void finish() = 0;
};

#endif //SERVER_ABSTRACT_ASYNC_HANDLER_H
