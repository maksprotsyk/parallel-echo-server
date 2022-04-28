//
// Created by mprotsyk on 27.04.22.
//

#ifndef SERVER_IO_OBJECT_H
#define SERVER_IO_OBJECT_H

#include <sys/socket.h>
#include <arpa/inet.h>

#include "abstract_async_handler.h"

class IOObject {
public:
    explicit IOObject(AbstractAsyncHandler* handler_, int fd_);
    [[nodiscard]] int getDescriptor() const;
    [[nodiscard]]  AbstractAsyncHandler* getHandler() const;

private:
    int fd;
    AbstractAsyncHandler* handler;

};

#endif //SERVER_IO_OBJECT_H
