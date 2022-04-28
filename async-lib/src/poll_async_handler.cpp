//
// Created by mprotsyk on 28.04.22.
//

#include "poll_async_handler.h"

PollAsyncHandler::PollAsyncHandler(size_t maxEvents_): maxEvents(maxEvents_), eventsNum(0), isFinished(false) {
}

bool PollAsyncHandler::addEvent(Event* event) {
    std::unique_lock lock(mapMutex);
    if (eventsNum == maxEvents) {
        return false;
    }

    pollfd pollEvent{};

    pollEvent.events = getMode(event->getType());
    pollEvent.fd = event->getDescriptor();

    PollEventData data;
    data.onReady = [event](){return event->makeReady();};
    data.onProcessed = [event](){return event->makeProcessed();};
    data.callback = event->getCallback();
    data.event = pollEvent;

    eventsData.emplace(event->getDescriptor(), data);

    eventsNum++;
    return true;
}

bool PollAsyncHandler::removeEvent(const Event* event) {
    std::unique_lock lock(mapMutex);
    if (eventsNum == 0) {
        return false;
    }
    return eventsData.erase(event->getDescriptor());
}


void PollAsyncHandler::runEventLoop() {
    auto eventDescriptors = new pollfd[maxEvents];
    auto actions = new PollEventData[maxEvents];
    while (!isFinished || eventsNum > 0) {
        size_t eventsCount = 0;
        {
            std::unique_lock lock(mapMutex);
            for (auto& [k, v]: eventsData) {
                eventDescriptors[eventsCount] = v.event;
                actions[eventsCount] = v;
                eventsCount++;
            }
        }

        auto count = poll(eventDescriptors, eventsCount, -1);
        if (count < 0) {
            throw std::runtime_error("Error in poll loop");
        }

        for (size_t i = 0; i < eventsCount; ++i) {
            if (! (eventDescriptors[i].events & eventDescriptors[i].revents)) {
                continue;
            }
            if (actions[i].onProcessed()) {
                removeEvent(eventDescriptors[i].fd);
                actions[i].callback();
                actions[i].onReady();
            }

        }
    }

    delete[] eventDescriptors;
    delete[] actions;

}

void PollAsyncHandler::finish() {
    isFinished = true;
}

bool PollAsyncHandler::detachEvent(const Event* event) {
    std::unique_lock lock(mapMutex);
    auto itr = eventsData.find(event->getDescriptor());
    if (itr == eventsData.end()) {
        return false;
    }
    itr->second.onReady = [](){return true;};
    itr->second.onProcessed = [](){return true;};

    return true;
}

short PollAsyncHandler::getMode(Event::Type type) {
    short res;
    switch (type) {
        case Event::Type::READY_IN:
            res = POLLIN;
            break;
        case Event::Type::READY_OUT:
            res = POLLOUT;
            break;
        case Event::Type::DISCONNECTION:
            res = POLLRDHUP;
            break;
    }
    return res;
}

bool PollAsyncHandler::removeEvent(int eventFd) {
    std::unique_lock lock(mapMutex);
    auto itr = eventsData.find(eventFd);
    if (itr == eventsData.end()) {
        return false;
    }
    eventsData.erase(itr);
    eventsNum--;

    return true;
}


