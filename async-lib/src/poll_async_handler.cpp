//
// Created by mprotsyk on 28.04.22.
//

#include "poll_async_handler.h"



PollAsyncHandler::PollAsyncHandler(size_t maxEvents_): maxEvents(maxEvents_), eventsNum(0), isFinished(false) {
    eventDescriptors = new pollfd[maxEvents];
}

bool PollAsyncHandler::addEvent(Event* event) {

    std::unique_lock lock(mutex);
    if (eventsNum == maxEvents) {
        return false;
    }


    PollEventData data;
    data.onReady = [event](){return event->makeReady();};
    data.onProcessed = [event](){return event->makeProcessed();};
    data.callback = event->getCallback();
    data.index = eventsNum;

    eventsData.emplace(event->getDescriptor(), data);

    pollfd pollEvent{};

    pollEvent.events = getMode(event->getType());
    pollEvent.fd = event->getDescriptor();

    eventDescriptors[eventsNum] = pollEvent;

    eventsNum++;
    return true;
}

bool PollAsyncHandler::removeEvent(const Event* event) {
    return removeEvent(event->getDescriptor());
}


void PollAsyncHandler::runEventLoop() {
    while (!isFinished || eventsNum > 0) {
        std::unique_lock lock(mutex);

        auto count = poll(eventDescriptors, eventsNum, -1);

        if (count < 0) {
            throw std::runtime_error("Error in poll loop");
        }

        std::vector<std::pair<PollEventData, int>> triggered;

        for (size_t i = 0; i < eventsNum; ++i) {
            if (! (eventDescriptors[i].revents)) {
                continue;
            }
            triggered.emplace_back(eventsData[eventDescriptors[i].fd], eventDescriptors[i].fd);

        }
        lock.unlock();

        for (const auto& [data, fd]: triggered) {
            if (data.onProcessed()) {
                removeEvent(fd);
                data.callback();
                data.onReady();
            }
        }


    }

}

void PollAsyncHandler::finish() {
    isFinished = true;
}

bool PollAsyncHandler::detachEvent(const Event* event) {
    std::unique_lock lock(mutex);
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
    std::unique_lock lock(mutex);
    if (eventsNum == 0) {
        return false;
    }
    auto itr = eventsData.find(eventFd);
    if (itr == eventsData.end()) {
        return false;
    }

    eventDescriptors[itr->second.index] = eventDescriptors[eventsNum - 1];

    eventsData[eventDescriptors[itr->second.index].fd].index = itr->second.index;
    eventsData.erase(eventFd);

    eventsNum--;
    return true;
}

bool PollAsyncHandler::addEvent(Event *event, const std::chrono::milliseconds &ms) {
    return false;
}

PollAsyncHandler::~PollAsyncHandler() {
    delete[] eventDescriptors;

}


