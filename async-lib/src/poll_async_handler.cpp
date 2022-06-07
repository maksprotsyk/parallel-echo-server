//
// Created by mprotsyk on 28.04.22.
//

#include "poll_async_handler.h"



PollAsyncHandler::PollAsyncHandler(size_t maxEvents_): maxEvents(maxEvents_), eventsNum(0), isFinished(false) {
    eventDescriptors = new pollfd[maxEvents];
}

bool PollAsyncHandler::addEvent(Event& event) {
    std::unique_lock lock(pollMutex);
    if (eventsNum == maxEvents) {
        return false;
    }
    eventsNum++;


    EventData eventData;
    eventData.fromEvent(event);

    mapMutex.lock();
    data.emplace(event.getDescriptor(), std::make_pair(eventData, eventsNum-1));
    mapMutex.unlock();

    pollfd pollEvent{};

    pollEvent.events = getMode(event.getType());
    pollEvent.fd = event.getDescriptor();

    eventDescriptors[eventsNum - 1] = pollEvent;


    return true;



}

bool PollAsyncHandler::removeEvent(const Event& event) {
    return removeEvent(event.getDescriptor());
}


void PollAsyncHandler::runEventLoop() {
    while (!isFinished || eventsNum > 0) {;
        std::unique_lock lock(pollMutex);
        auto count = poll(eventDescriptors, eventsNum, 0);

        if (count < 0) {
            throw std::runtime_error("Error in poll loop");
        }

        std::vector<EventData> toPerform;
        mapMutex.lock();
        for (size_t i = 0; i < eventsNum; ++i) {
            if (!(eventDescriptors[i].revents & eventDescriptors[i].events)) {
                continue;
            }
            int fd = eventDescriptors[i].fd;
            auto itr = data.find(fd);
            if (itr == data.end()) {
                continue;
            }
            toPerform.emplace_back(itr->second.first);
        }
        mapMutex.unlock();
        lock.unlock();

        for (auto& eventData: toPerform) {
            if (eventData.onProcessed()) {
                removeEvent(eventData.fd);
                eventData.callback();
                eventData.onReady();
            } else {
                removeEvent(eventData.fd);
            }
        }

        queueMutex.lock();
        mapMutex.lock();
        std::vector<EventData> timeouts;
        while (!deadlines.empty() && deadlines.top().deadline < std::chrono::system_clock::now()) {
            auto itr = data.find(deadlines.top().fd);
            if (itr == data.end()) {
                continue;
            }
            timeouts.emplace_back(itr->second.first);
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

void PollAsyncHandler::finish() {
    isFinished = true;
}

bool PollAsyncHandler::detachEvent(const Event& event) {
    std::unique_lock lock(mapMutex);
    auto itr = data.find(event.getDescriptor());
    if (itr == data.end()) {
        return false;
    }
    auto& eventData = itr->second.first;
    eventData.fromDetachedEvent(event);

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
    std::unique_lock lock(pollMutex);
    if (eventsNum == 0) {
        return false;
    }

    mapMutex.lock();
    auto itr = data.find(eventFd);
    if (itr == data.end()) {
        return false;
    }


    eventDescriptors[itr->second.second] = eventDescriptors[eventsNum - 1];

    data[eventDescriptors[itr->second.second].fd].second = itr->second.second;
    data.erase(eventFd);

    mapMutex.unlock();

    eventsNum--;
    return true;
}

bool PollAsyncHandler::addEvent(Event& event, const std::chrono::milliseconds &ms) {
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

PollAsyncHandler::~PollAsyncHandler() {
    delete[] eventDescriptors;
}


