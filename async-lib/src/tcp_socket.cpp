//
// Created by mprotsyk on 27.04.22.
//

#include "tcp_socket.h"


TcpSocket::TcpSocket(AbstractAsyncHandler *handler): IOObject(handler, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)) {
    if (getDescriptor() < 0) {
        throw std::runtime_error("Can't create socket");
    }
}
