//
// Created by maksprotsyk on 05.06.22.
//

#ifndef ASYNC_LIB_PROMISE_H
#define ASYNC_LIB_PROMISE_H

#include <functional>
#include <optional>

#include <sys/eventfd.h>

#include "event.h"
#include "abstract_async_handler.h"


template<class T>
class Promise {
public:
    enum class State {
        PENDING, FULLFILL, REJECTED
    };

    using RejectFunction = std::function<void(int)>;

    template<class... Args>
            Promise(
            AbstractAsyncHandler* handler,
            std::function<void(std::function<void(Args&&...)>, RejectFunction)> function
            );

    Promise(AbstractAsyncHandler* handler);

    auto getState();
    T getValue();
    int getError();
    void wait();

    template<class U>
    Promise<U> then(std::function<U(T)> acceptor, std::function<int(int)> rejector);

    template<class U>
    Promise<U> then(std::function<U(T)> acceptor);

    Promise<T> catch_error(std::function<int(int)> rejector);
private:
    class PromiseData {
    public:
        std::shared_ptr<T> value = nullptr;
        int error = 0;
        State state;
        std::shared_ptr<Event> finishEvent = nullptr;
        std::mutex stateMutex;
        std::condition_variable stateCv;

        template <class... Args> void makeFullFilled(Args&&... args);
        void makeRejected(int error);
    };

    std::shared_ptr<PromiseData> data;
    AbstractAsyncHandler* handler;

};

template<class T>
template<class... Args>
void Promise<T>::PromiseData::makeFullFilled(Args&&... args) {
    std::unique_lock lock(stateMutex);
    value = std::make_shared<T>(args...);
    int64_t num = 1;
    write(finishEvent->getDescriptor(), &num, sizeof(int64_t));
    state = State::FULLFILL;
    stateCv.notify_all();
}

template<class T>
void Promise<T>::PromiseData::makeRejected(int error_) {
    std::unique_lock lock(stateMutex);
    error = error_;
    int64_t num = 1;
    write(finishEvent->getDescriptor(), &num, sizeof(int64_t));
    state = State::REJECTED;
    stateCv.notify_all();
}


template<class T>
template<class U>
Promise<U> Promise<T>::then(std::function<U(T)> acceptor, std::function<int(int)> rejector) {
    std::unique_lock lock(data->stateMutex);
    Promise<U> promise(handler);
    auto dataPtr = promise.data;
    if (data->finishEvent->getState() == Event::State::SCHEDULED) {
        auto newData = data;
        auto callback = data->finishEvent->getCallback();
        auto newCallback = [dataPtr, newData, callback, acceptor, rejector]() {
            callback();
            if (newData->state == State::FULLFILL) {
                dataPtr->makeFullFilled(acceptor(*newData->value));
            } else if (newData->state == State::REJECTED) {
                dataPtr->makeRejected(rejector(newData->error));
            } else {
                throw std::runtime_error("Promise is still being processed");
            }
        };
        data->finishEvent->setCallback(newCallback);
    } else {
        if (data->state == State::FULLFILL) {
            dataPtr->makeFullFilled(acceptor(*data->value));
        } else if (data->state == State::REJECTED) {
            dataPtr->makeRejected(rejector(data->error));
        } else {
            throw std::runtime_error("Promise is still being processed");
        }
    }
    return promise;

}

template<class T>
template<class... Args>
Promise<T>::Promise(
        AbstractAsyncHandler* handler_,
        std::function<void(std::function<void(Args &&...)>, RejectFunction)> function
        ): data(std::make_shared<PromiseData>()), handler(handler_) {
    data->state = State::PENDING;
    data->finishEvent = std::make_shared<Event>(
            handler,
            eventfd(0, EFD_NONBLOCK),
            Event::Type::READY_IN
            );
    data->finishEvent->schedule();
    auto new_data = data;
    function(
            [new_data](Args &&... args) {new_data->makeFullFilled(args...);},
            [new_data](int error) {new_data->makeRejected(error);}
            );

}

template<class T>
auto Promise<T>::getState() {
    std::unique_lock lock(data->stateMutex);
    return data->state;
}

template<class T>
T Promise<T>::getValue() {
    std::unique_lock lock(data->stateMutex);
    if (data->state != State::FULLFILL) {
        throw std::runtime_error("Promise is not fullfilled");
    }
    return *data->value;
}

template<class T>
int Promise<T>::getError() {
    std::unique_lock lock(data->stateMutex);
    if (data->state != State::REJECTED) {
        throw std::runtime_error("Promise is not rejected");
    }
    return data->error;
}

template<class T>
void Promise<T>::wait() {
    std::unique_lock lock(data->stateMutex);
    while (data->state == State::PENDING) {
        data->stateCv.wait(lock);
    }
}

template<class T>
Promise<T>::Promise(AbstractAsyncHandler* handler_): handler(handler_), data(std::make_shared<PromiseData>()) {
    data->state = State::PENDING;
    data->finishEvent = std::make_shared<Event>(handler, eventfd(0, EFD_NONBLOCK), Event::Type::READY_IN);
    data->finishEvent->schedule();
}

template<class T>
template<class U>
Promise<U> Promise<T>::then(std::function<U(T)> acceptor) {
    std::function rejector = [](int error){return error;};
    return then(acceptor, rejector);
}

template<class T>
Promise<T> Promise<T>::catch_error(std::function<int(int)> rejector) {
    std::function acceptor = [](T val){return val;};
    return then(acceptor, rejector);
}

#endif //ASYNC_LIB_PROMISE_H
