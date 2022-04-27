//
// Created by mprotsyk on 27.04.22.
//
#include "abstract_event.h"

AbstractEvent::AbstractEvent(AbstractAsyncHandler *handler_, int fd_, AbstractEvent::Type type_)
: handler(handler_),
  fd(fd_),
  type(type_),
  state(State::INIT)
{
    callback = [](){};
}

void AbstractEvent::setCallback(const std::function<void()> &callback_) {
    callback = callback_;

}

std::function<void()> AbstractEvent::getCallback() const {
    return callback;
}

AbstractEvent::Type AbstractEvent::getType() const {
    return type;
}

int AbstractEvent::getDescriptor() const {
    return fd;
}


