//
// Created by mprotsyk on 27.04.22.
//
#include "event.h"

Event::Event(AbstractAsyncHandler *handler_, int fd_, Event::Type type_)
: handler(handler_),
  fd(fd_),
  type(type_),
  state(State::INIT)
{
    callback = [](){};
    timeoutCallback = [](){};
}

void Event::setCallback(const std::function<void()> &callback_) {
    callback = callback_;

}

std::function<void()> Event::getCallback() const {
    return callback;
}

Event::Type Event::getType() const {
    return type;
}

int Event::getDescriptor() const {
    return fd;
}


Event::~Event() {
    std::unique_lock lock(mutex);
    if (state == State::SCHEDULED) {
        myCancel();
    }
}

bool Event::cancel() {
    std::unique_lock lock(mutex);
    bool res = myCancel();
    if (res) {
        cv.notify_one();
    }
    return res;
}

bool Event::schedule() {
    std::unique_lock lock(mutex);
    if (state != State::INIT) {
        return false;
    }
    state = State::SCHEDULED;
    handler->addEvent(*this);
    return true;
}


bool Event::wait() {
    std::unique_lock lock(mutex);
    while (state == State::SCHEDULED || state == State::PROCESSED) {
        cv.wait(lock);
    }
    return state == State::READY;
}

bool Event::makeReady() {
    std::unique_lock lock(mutex);
    if (state != State::PROCESSED) {
        return false;
    }
    state = State::READY;
    cv.notify_one();
    return true;
}

bool Event::detach() {
    std::unique_lock lock(mutex);
    if (state != State::SCHEDULED) {
        return false;
    }
    if (! handler->detachEvent(*this)) {
        return false;
    }
    state = State::DETACHED;
    cv.notify_one();
    return true;

}

Event::State Event::getState() const {
    std::unique_lock lock(mutex);
    return state;
}

bool Event::myCancel() {
    if (state != State::SCHEDULED) {
        return false;
    }
    state = State::CANCELED;
    handler->removeEvent(*this);
    return true;
}

bool Event::makeProcessed() {
    std::unique_lock lock(mutex);
    if (state != State::SCHEDULED) {
        return false;
    }
    state = State::PROCESSED;
    return true;
}

bool Event::makeTimeout() {
    std::unique_lock lock(mutex);
    if (state != State::PROCESSED_TIMEOUT) {
        return false;
    }
    state = State::TIMEOUT;
    cv.notify_one();
    return true;
}


bool Event::schedule(const std::chrono::milliseconds &ms) {
    std::unique_lock lock(mutex);
    if (state != State::INIT) {
        return false;
    }
    state = State::SCHEDULED;
    handler->addEvent(*this, ms);
    return true;
}

void Event::setTimeoutCallback(const std::function<void()> &timeoutCallback_) {
    timeoutCallback = timeoutCallback_;

}

std::function<void()> Event::getTimeoutCallback() const {
    return timeoutCallback;
}

bool Event::makeProcessedTimeout() {
    std::unique_lock lock(mutex);
    if (state != State::SCHEDULED) {
        return false;
    }
    state = State::PROCESSED_TIMEOUT;
    return true;
}


