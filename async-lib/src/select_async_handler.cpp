//
// Created by maksprotsyk on 05.06.22.
//

#include "select_async_handler.h"

SelectAsyncHandler::SelectAsyncHandler(): eventsNum(0), isFinished(false), sets(3) {

}

bool SelectAsyncHandler::addEvent(Event& event) {

    EventData eventData;
    eventData.fromEvent(event);

    size_t type = getType(event.getType());

    mapMutex.lock();
    data.emplace(event.getDescriptor(), eventData);
    mapMutex.unlock();

    selectMutex.lock();
    eventsNum++;
    sets[type].insert(event.getDescriptor());
    selectMutex.unlock();


    return true;
}

bool SelectAsyncHandler::addEvent(Event& event, const std::chrono::milliseconds &ms) {
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

bool SelectAsyncHandler::removeEvent(const Event& event) {
    return removeEvent(event.getDescriptor());
}

bool SelectAsyncHandler::detachEvent(const Event& event) {
    std::unique_lock lock(mapMutex);
    auto itr = data.find(event.getDescriptor());
    if (itr == data.end()) {
        return false;
    }
    auto& eventData = itr->second;
    eventData.fromDetachedEvent(event);

    return true;
}


void SelectAsyncHandler::runEventLoop() {
    while (!isFinished || eventsNum > 0) {
        std::array<fd_set, 3> select_sets{};

        for (auto& set: select_sets) {
            FD_ZERO(&set);
        }

        selectMutex.lock();
        for (size_t i = 0; i < select_sets.size(); i++) {
            for (const auto& fd: sets[i]) {
                FD_SET(fd, &select_sets[i]);
            }
        }
        selectMutex.unlock();

        std::unique_lock lock(mapMutex);
        if (data.empty()) {
            continue;
        }
        int nfds = data.rbegin()->first + 1;
        timeval tv{};
        tv.tv_sec = 0;
        tv.tv_usec = 1000;
        auto count = select(
                nfds,
                &select_sets[0],
                &select_sets[1],
                &select_sets[2],
                &tv
                );
        if (count < 0) {
            throw std::runtime_error("Error in epoll loop");
        }

        std::vector<EventData> toPerform;
        selectMutex.lock();
        for (size_t i = 0; i < select_sets.size(); i++) {
            for (const auto& fd: sets[i]) {
                if (!FD_ISSET(fd, &select_sets[i])) {
                    continue;
                }
                auto itr = data.find(fd);
                if (itr == data.end()) {
                    continue;
                }
                toPerform.emplace_back(itr->second);
            }
        }

        selectMutex.unlock();
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

void SelectAsyncHandler::finish() {
    isFinished = true;
}



bool SelectAsyncHandler::removeEvent(int eventFd) {
    std::unique_lock lock(mapMutex);
    if (!data.erase(eventFd)) {
        return false;
    }
    lock.unlock();

    selectMutex.lock();
    eventsNum--;
    for (auto& set: sets) {
        set.erase(eventFd);
    }
    selectMutex.unlock();

    return true;
}

size_t SelectAsyncHandler::getType(Event::Type type) {
    switch (type) {
        case Event::Type::READY_IN:
            return 0;
        case Event::Type::READY_OUT:
            return 1;
        case Event::Type::DISCONNECTION:
            return 2;
        default:
            throw std::runtime_error("Invalid type");
    }
}
