//
// Created by mprotsyk on 27.04.22.
//

#include "tcp_socket.h"


TcpSocket::TcpSocket(AbstractAsyncHandler *handler)
: IOObject(handler, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)) {
}

IOObject::Error TcpSocket::read(std::vector<char> &data) {
    return IOObject::read(data);
}

void TcpSocket::asyncRead(
        std::vector<char> &data,
        std::function<void(IOObject &, std::vector<char> &, Error)> callback
        ) {
    IOObject::asyncRead(data, callback);
}

IOObject::Error TcpSocket::write(std::vector<char> &data) {
    return IOObject::write(data);
}

void
TcpSocket::asyncWrite(
        std::vector<char> &data,
        std::function<void(IOObject &, std::vector<char> &, Error
        )> callback) {
    IOObject::asyncWrite(data, callback);
}

IOObject::Error TcpSocket::bind(const std::string &ip, int port) {
    return IOObject::bind(ip, port);
}

IOObject::Error TcpSocket::listen(size_t maxPendingConnections) {
    return IOObject::listen(maxPendingConnections);
}

IOObject::Error TcpSocket::connect(const std::string &ip, int port) {
    return IOObject::connect(ip, port);
}

void TcpSocket::asyncConnect(
        const std::string &ip,
        int port,
        std::function<void(IOObject &, Error)> callback
        ) {
    IOObject::asyncConnect(ip, port, callback);
}

IOObject::Error TcpSocket::close() {
    return IOObject::close();
}

void TcpSocket::asyncAccept(const std::function<void(IOObject, Error)> &callback) {
    IOObject::asyncAccept(callback);
}

std::pair<IOObject, IOObject::Error> TcpSocket::accept() {
    return IOObject::accept();
}
