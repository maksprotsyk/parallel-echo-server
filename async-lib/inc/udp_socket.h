//
// Created by maksprotsyk on 06.06.22.
//

#ifndef ASYNC_LIB_UDP_SOCKET_H
#define ASYNC_LIB_UDP_SOCKET_H

#include <sys/socket.h>
#include <arpa/inet.h>

#include "io_object.h"
#include "abstract_async_handler.h"

class UDPSocket: private IOObject {
public:
    explicit UDPSocket(AbstractAsyncHandler* handler);

    Error read(std::vector<char>& data) override;
    void asyncRead(
            std::vector<char>& data,
            std::function<void(IOObject&, std::vector<char>&, Error)> callback) override;
    Error write(std::vector<char>& data) override;
    void asyncWrite(std::vector<char>& data, std::function<void(IOObject&, std::vector<char>&, Error)> callback) override;
    Error bind(const std::string& ip, int port) override;

    Error close() override;

    void asyncSendTo(
            const std::string& ip,
            int port,
            std::vector<char>& data,
            std::function<void(IOObject&, std::vector<char>&, Error)> callback) override;
    Error sendTo(
            const std::string& ip,
            int port,
            std::vector<char>& data) override;

    std::pair<Address, Error> recvFrom(
            std::vector<char>& data
    ) override;
    void asyncRecvFrom(
            std::vector<char>& data,
            std::function<void(IOObject&, Address, std::vector<char>&, Error)> callback
    ) override;


};


#endif //ASYNC_LIB_UDP_SOCKET_H
