//
// Created by mprotsyk on 27.04.22.
//

#include "event.h"

Event::Event(AbstractAsyncHandler* handler, int fd, Type type): AbstractEvent(handler, fd, type) {

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
    handler->addEvent(this);
    return true;
}


bool Event::wait() {
    std::unique_lock lock(mutex);
    while (state == State::SCHEDULED) {
        cv.wait(lock);
    }
    return state == State::READY;
}

bool Event::makeReady() {
    std::unique_lock lock(mutex);
    if (state != State::SCHEDULED) {
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
    if (! handler->detachEvent(this)) {
        return false;
    }
    state = State::DETACHED;
    cv.notify_one();
    return true;

}

AbstractEvent::State Event::getState() const {
    std::unique_lock lock(mutex);
    return state;
}

bool Event::myCancel() {
    if (state != State::SCHEDULED) {
        return false;
    }
    state = State::CANCELED;
    handler->removeEvent(this);
    return true;
}

