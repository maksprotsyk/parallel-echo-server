
#include "timer_event.h"
#include <sys/timerfd.h>

TimerEvent::TimerEvent(AbstractAsyncHandler *handler_, int fd_,
                       const std::chrono::seconds &s, Type type_)
        : Event(handler_, fd_, type_) {
    struct itimerspec ts{};

    ts.it_interval.tv_sec = s.count();
    ts.it_interval.tv_nsec = 0;
    ts.it_value.tv_sec = s.count();
    ts.it_value.tv_nsec = 0;

    timerfd_settime(fd, 0, &ts, nullptr);
}

