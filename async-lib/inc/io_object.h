//
// Created by mprotsyk on 27.04.22.
//

#ifndef SERVER_IO_OBJECT_H
#define SERVER_IO_OBJECT_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

#include "event.h"
#include "abstract_async_handler.h"

struct Address {
    std::string ip{};
    int port{};
};

class IOObject {
public:
    enum class Error {
        SUCCESS, ERROR
    };

    explicit IOObject(AbstractAsyncHandler* handler_, int fd_);
    [[nodiscard]] virtual int getDescriptor() const;
    [[nodiscard]]  virtual AbstractAsyncHandler* getHandler() const;

    virtual Error read(std::vector<char>& data);
    virtual void asyncRead(std::vector<char>& data, std::function<void(IOObject&, std::vector<char>&, Error)> callback);
    virtual Error write(std::vector<char>& data);
    virtual void asyncWrite(std::vector<char>& data, std::function<void(IOObject&, std::vector<char>&, Error)> callback);
    virtual Error bind(const std::string& ip, int port);
    virtual Error listen(size_t maxPendingConnections);

    virtual void asyncSendTo(
            const std::string& ip,
            int port,
            std::vector<char>& data, std::function<void(IOObject&, std::vector<char>&, Error)> callback);
    virtual Error sendTo(
            const std::string& ip,
            int port,
            std::vector<char>& data);

    virtual std::pair<Address, Error> recvFrom(
            std::vector<char>& data
            );
    virtual void asyncRecvFrom(
            std::vector<char>& data,
            std::function<void(IOObject&, Address, std::vector<char>&, Error)> callback
            );

    virtual Error connect(const std::string& ip, int port);
    virtual void asyncConnect(const std::string& ip, int port, std::function<void(IOObject&, Error)> callback);

    virtual Error close();

    virtual void asyncAccept(const std::function<void(IOObject, Error)>& callback);
    virtual std::pair<IOObject, Error> accept();

private:
    int fd;
    AbstractAsyncHandler* handler;

    std::function<void()> readCallback(
            std::vector<char>& data,
            std::function<void(IOObject&, std::vector<char>&, Error)> callback);

    std::function<void()> writeCallback(
            std::vector<char>& data,
            std::function<void(IOObject&, std::vector<char>&, Error)> callback);

    std::function<void()> sendToCallback(
            const std::string& ip,
            int port,
            std::vector<char>& data,
            std::function<void(IOObject&, std::vector<char>&, Error)> callback);

    std::function<void()> recvFromCallback(
            std::vector<char>& data,
            std::function<void(IOObject&, Address, std::vector<char>&, Error)> callback);

    std::function<void()> acceptCallback(std::function<void(IOObject, Error)> callback);

};

#endif //SERVER_IO_OBJECT_H
