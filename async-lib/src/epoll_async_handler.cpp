//
// Created by mprotsyk on 27.04.22.
//

#include "epoll_async_handler.h"

EpollAsyncHandler::EpollAsyncHandler(size_t maxEvents_)
: maxEvents(maxEvents_), eventsNum(0), isFinished(false) {
    events = new epoll_event[maxEvents_];
    epollFd = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd < 0) {
        throw std::runtime_error("Can't create epoll structure");
    }
}

bool EpollAsyncHandler::addEvent(Event& event) {
    if (eventsNum++ == maxEvents) {
        eventsNum--;
        return false;
    }

    epoll_event epollEvent{};

    epollEvent.events = getMode(event.getType());
    epollEvent.data.fd = event.getDescriptor();


    EventData eventData;
    eventData.fromEvent(event);

    mapMutex.lock();
    data.emplace(event.getDescriptor(), eventData);
    mapMutex.unlock();

    epollMutex.lock();
    int res = epoll_ctl(epollFd, EPOLL_CTL_ADD, event.getDescriptor(), &epollEvent);
    epollMutex.unlock();

    if (res < 0) {
        perror("Error: ");
        throw std::runtime_error("Can't add the event to epoll struct");
    }
    return true;
}

bool EpollAsyncHandler::removeEvent(const Event& event) {
    return removeEvent(event.getDescriptor());
}

EpollAsyncHandler::~EpollAsyncHandler() {
    close(epollFd);
    delete[] events;
}

void EpollAsyncHandler::runEventLoop() {
    while (!isFinished || eventsNum > 0) {
        epollMutex.lock();
        auto count = epoll_wait(epollFd, events, static_cast<int>(maxEvents), 0);
        epollMutex.unlock();
        if (count < 0) {
            throw std::runtime_error("Error in epoll loop");
        }

        std::vector<EventData> toPerform;
        mapMutex.lock();
        for (size_t i = 0; i < count; ++i) {
            int fd = events[i].data.fd;
            auto itr = data.find(fd);
            if (itr == data.end()) {
                continue;
            }
            toPerform.emplace_back(itr->second);
        }
        mapMutex.unlock();

        for (auto& eventData: toPerform) {
            if (eventData.onProcessed()) {
                removeEvent(eventData.fd);
                eventData.callback();
                eventData.onReady();
            } else {
                removeEvent(eventData.fd);
            }
        }

        mapMutex.lock();
        queueMutex.lock();
        std::vector<EventData> timeouts;
        while (!deadlines.empty() && deadlines.top().deadline < std::chrono::system_clock::now()) {
            auto itr = data.find(deadlines.top().fd);
            if (itr == data.end()) {
                continue;
            }
            timeouts.emplace_back(itr->second);
            deadlines.pop();
        }
        queueMutex.unlock();
        mapMutex.unlock();

        for (auto& eventData: timeouts) {
            if (eventData.onProcessedTimeout()) {
                removeEvent(eventData.fd);
                eventData.timeoutCallback();
                eventData.onTimeout();
            } else {
                removeEvent(eventData.fd);
            }
        }


    }

}

void EpollAsyncHandler::finish() {
    isFinished = true;
}

bool EpollAsyncHandler::detachEvent(const Event& event) {

    std::unique_lock lock(mapMutex);
    auto itr = data.find(event.getDescriptor());
    if (itr == data.end()) {
        return false;
    }
    auto& eventData = itr->second;

    eventData.fromDetachedEvent(event);

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
    epollMutex.lock();
    int res = epoll_ctl(epollFd, EPOLL_CTL_DEL, eventFd, nullptr);
    epollMutex.unlock();
    if (res < 0) {
        return false;
    }

    std::unique_lock lock(mapMutex);
    if (!data.erase(eventFd)) {
        return false;
    }
    eventsNum--;
    return true;


}

bool EpollAsyncHandler::addEvent(Event& event, const std::chrono::milliseconds &ms) {
    if (!addEvent(event)) {
        return false;
    }

    std::unique_lock lock(queueMutex);
    Deadline deadline;
    deadline.deadline = std::chrono::system_clock::now() + ms;
    deadline.fd = event.getDescriptor();
    deadlines.emplace(deadline);
    return true;
}


