//
// Created by mprotsyk on 27.04.22.
//

#ifndef SERVER_SOCKET_SERVER_H
#define SERVER_SOCKET_SERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>

#include "io_object.h"
#include "abstract_async_handler.h"

class SocketServer: public IOObject {
public:
    explicit SocketServer(AbstractAsyncHandler* handler);
};

#endif //SERVER_SOCKET_SERVER_H
