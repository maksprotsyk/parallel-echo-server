//
// Created by maksprotsyk on 05.06.22.
//

#ifndef SERVER_EVENT_DATA_H
#define SERVER_EVENT_DATA_H

#include <functional>
#include "event.h"

struct EventData {
    std::function<void()> callback;
    std::function<bool()> onProcessed;
    std::function<bool()> onReady;
    std::function<bool()> onTimeout;
    std::function<void()> timeoutCallback;
    std::function<bool()> onProcessedTimeout;
    int fd{};

    void fromEvent(Event& event);
    void fromDetachedEvent(const Event& event);

};

#endif //SERVER_EVENT_DATA_H
