//
// Created by mprotsyk on 28.04.22.
//

#ifndef SERVER_ECHO_SERVER_H
#define SERVER_ECHO_SERVER_H

#include <iostream>

#include "socket_server.h"
#include "operations.h"
#include "abstract_async_handler.h"

class EchoServer {
public:
    EchoServer(AbstractAsyncHandler* handler, const std::string& ip, int port);

    void readFunction(IOObject& usedObject, std::vector<char>& newData, Operations::Error error);
    void writeFunction(IOObject& usedObject, std::vector<char>& newData, Operations::Error error);
    void acceptFunction(IOObject newObject, Operations::Error error);
    void deleteFunction(IOObject obj);
private:
    SocketServer server;
    std::unordered_map<int, IOObject> connections;
    std::unordered_map<int, std::vector<char>> buffers;



};

#endif //SERVER_ECHO_SERVER_H
