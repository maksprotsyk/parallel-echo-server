//
// Created by maksprotsyk on 05.06.22.
//

#include "event_data.h"

void EventData::fromEvent(Event& event) {
    onReady = [&event](){return event.makeReady();};
    onProcessed = [&event](){return event.makeProcessed();};
    callback = [&event]() {event.getCallback()();};
    timeoutCallback = [&event]() {event.getTimeoutCallback()();};
    onProcessedTimeout = [&event]() {return event.makeProcessedTimeout();};
    onTimeout = [&event]() {return event.makeTimeout();};
    fd = event.getDescriptor();
}

void EventData::fromDetachedEvent(const Event& event) {
    onReady = []() { return true; };
    onProcessed = []() { return true; };
    onTimeout = []() { return true; };
    onProcessedTimeout = []() { return true; };
    callback = event.getCallback();
    timeoutCallback = event.getTimeoutCallback();
    fd = event.getDescriptor();
}
