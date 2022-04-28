//
// Created by mprotsyk on 27.04.22.
//

#ifndef SERVER_ASYNC_FILE_H
#define SERVER_ASYNC_FILE_H

#include <unistd.h>

#include "event.h"
#include "epoll_async_handler.h"

using Bytes=std::vector<char>;

class AsyncFile {
public:
    enum class OperationResult {
        SUCCESS, ERROR
    };
    static constexpr size_t READ_BUFFER_SIZE = 512;
    AsyncFile(EpollAsyncHandler& handler_, int fd_);
    void scheduleRead(const std::function<void(const std::shared_ptr<Bytes>& bytes, OperationResult)>& callback);
    void scheduleWrite(const std::shared_ptr<Bytes>& bytes, const std::function<void(OperationResult)>& callback);
    void closeFile() const;

protected:
    EpollAsyncHandler& handler;
    int fd;
    std::unique_ptr<Event> readEvent = nullptr;
    std::unique_ptr<Event> writeEvent = nullptr;

};


#endif //SERVER_ASYNC_FILE_H
