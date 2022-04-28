//
// Created by mprotsyk on 27.04.22.
//

#include "epoll_async_handler.h"

struct EpollEventData {
    std::function<void()> callback;
    std::function<bool()> onProcessed;
    std::function<bool()> onReady;
    int fd;
};


EpollAsyncHandler::EpollAsyncHandler(size_t maxEvents_): maxEvents(maxEvents_), eventsNum(0), isFinished(false) {
    events = new epoll_event[maxEvents_];
    fd = epoll_create1(EPOLL_CLOEXEC);
    if (fd < 0) {
        throw std::runtime_error("Can't create epoll structure");
    }
}

bool EpollAsyncHandler::addEvent(Event* event) {
    if (eventsNum == maxEvents) {
        return false;
    }

    epoll_event epollEvent{};

    epollEvent.events = getMode(event->getType());

    auto data = new EpollEventData;
    data->onReady = [event](){return event->makeReady();};
    data->onProcessed = [event](){return event->makeProcessed();};
    data->callback = event->getCallback();
    data->fd = event->getDescriptor();
    epollEvent.data.ptr = reinterpret_cast<void*>(data);


    int res = epoll_ctl(fd, EPOLL_CTL_ADD, event->getDescriptor(), &epollEvent);
    if (res < 0) {
        perror("Error: ");
        throw std::runtime_error("Can't add the event to epoll struct");
    }
    eventsNum++;
    return true;
}

bool EpollAsyncHandler::removeEvent(const Event* event) {
    return removeEvent(event->getDescriptor());
}

EpollAsyncHandler::~EpollAsyncHandler() {
    close(fd);
    delete[] events;
}

void EpollAsyncHandler::runEventLoop() {
    while (!isFinished || eventsNum > 0) {
        auto count = epoll_wait(fd, events, static_cast<int>(maxEvents), 0);
        if (count < 0) {
            throw std::runtime_error("Error in epoll loop");
        }


        for (size_t i = 0; i < count; ++i) {
            auto data = reinterpret_cast<EpollEventData*>(events[i].data.ptr);
            if (data->onProcessed()) {
                removeEvent(data->fd);
                data->callback();
                data->onReady();
            }

        }
    }

}

void EpollAsyncHandler::finish() {
    isFinished = true;
}

bool EpollAsyncHandler::detachEvent(const Event* event) {
    epoll_event epollEvent{};

    epollEvent.events = getMode(event->getType());

    auto data = new EpollEventData;
    data->onReady = [](){return true;};
    data->onProcessed = [](){return true;};
    data->callback = event->getCallback();
    data->fd = event->getDescriptor();
    epollEvent.data.ptr = reinterpret_cast<void*>(data);

    int res = epoll_ctl(fd, EPOLL_CTL_MOD, event->getDescriptor(), &epollEvent);
    if (res < 0) {
        throw std::runtime_error("Can't add the event to epoll struct");
    }
    return true;
}

int EpollAsyncHandler::getMode(Event::Type type) {
    int res;
    switch (type) {
        case Event::Type::READY_IN:
            res = EPOLLIN;
            break;
        case Event::Type::READY_OUT:
            res = EPOLLOUT;
            break;
        case Event::Type::DISCONNECTION:
            res = EPOLLRDHUP;
            break;
    }
    res |= EPOLLONESHOT;
    return res;
}

bool EpollAsyncHandler::removeEvent(int eventFd) {
    int res = epoll_ctl(fd, EPOLL_CTL_DEL, eventFd, nullptr);
    if (res < 0) {
        return false;
    }
    eventsNum--;
    return true;
}


