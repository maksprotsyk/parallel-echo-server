
#ifndef ASYNC_LIB_TIMER_EVENT_H
#define ASYNC_LIB_TIMER_EVENT_H

#include "event.h"

class TimerEvent : public Event {
public:
    TimerEvent(AbstractAsyncHandler *, int, const std::chrono::seconds &, Type);
    TimerEvent(const Event &event) = delete;

private:
};


#endif //ASYNC_LIB_TIMER_EVENT_H
