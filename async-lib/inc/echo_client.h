//
// Created by mprotsyk on 28.04.22.
//

#ifndef SERVER_ECHO_CLIENT_H
#define SERVER_ECHO_CLIENT_H


#include <iostream>

#include "tcp_socket.h"
#include "operations.h"
#include "abstract_async_handler.h"
#include "logging.h"

class EchoClient {
public:
    EchoClient(AbstractAsyncHandler* handler, const std::string& ip, int port);

    void run();
    ~EchoClient();

private:
    TcpSocket client;
    std::vector<char> buffer;

};


#endif //SERVER_ECHO_CLIENT_H
