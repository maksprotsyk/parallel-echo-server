//
// Created by mprotsyk on 27.04.22.
//

#ifndef SERVER_TCP_SOCKET_H
#define SERVER_TCP_SOCKET_H

#include <sys/socket.h>
#include <arpa/inet.h>

#include "io_object.h"
#include "abstract_async_handler.h"

class TcpSocket: private IOObject {
public:
    explicit TcpSocket(AbstractAsyncHandler* handler);

    Error read(std::vector<char>& data) override;
    void asyncRead(
            std::vector<char>& data,
            std::function<void(IOObject&, std::vector<char>&, Error)> callback) override;
    Error write(std::vector<char>& data) override;
    void asyncWrite(std::vector<char>& data, std::function<void(IOObject&, std::vector<char>&, Error)> callback) override;
    Error bind(const std::string& ip, int port) override;
    Error listen(size_t maxPendingConnections) override;

    Error connect(const std::string& ip, int port) override;
    void asyncConnect(const std::string& ip, int port, std::function<void(IOObject&, Error)> callback) override;

    Error close() override;

    void asyncAccept(const std::function<void(IOObject, Error)>& callback) override;
    std::pair<IOObject, Error> accept() override;
};

#endif //SERVER_TCP_SOCKET_H
