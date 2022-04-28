//
// Created by mprotsyk on 27.04.22.
//

#ifndef SERVER_TCP_SOCKET_H
#define SERVER_TCP_SOCKET_H

#include <sys/socket.h>
#include <arpa/inet.h>

#include "io_object.h"
#include "abstract_async_handler.h"

class TcpSocket: public IOObject {
public:
    explicit TcpSocket(AbstractAsyncHandler* handler);
};

#endif //SERVER_TCP_SOCKET_H
