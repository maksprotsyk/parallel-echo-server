//
// Created by mprotsyk on 28.04.22.
//

#ifndef SERVER_ECHO_SERVER_H
#define SERVER_ECHO_SERVER_H

#include <iostream>

#include "tcp_socket.h"
#include "abstract_async_handler.h"
#include "logging.h"

class EchoServer {
public:
    EchoServer(AbstractAsyncHandler* handler, const std::string& ip, int port);

    void readFunction(IOObject& usedObject, std::vector<char>& newData, IOObject::Error error);
    void writeFunction(IOObject& usedObject, std::vector<char>& newData, IOObject::Error error);
    void acceptFunction(IOObject newObject, IOObject::Error error);
    void deleteFunction(IOObject obj);
private:
    TcpSocket server;
    std::unordered_map<int, IOObject> connections;
    std::unordered_map<int, std::vector<char>> buffers;



};

#endif //SERVER_ECHO_SERVER_H
