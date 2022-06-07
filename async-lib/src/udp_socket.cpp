//
// Created by maksprotsyk on 06.06.22.
//

#include "udp_socket.h"

UDPSocket::UDPSocket(AbstractAsyncHandler *handler)
: IOObject(handler, socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP)){

}

IOObject::Error UDPSocket::read(std::vector<char> &data) {
    return IOObject::read(data);
}

void
UDPSocket::asyncRead(std::vector<char> &data, std::function<void(IOObject &, std::vector<char> &, Error)> callback) {
    IOObject::asyncRead(data, callback);
}

IOObject::Error UDPSocket::write(std::vector<char> &data) {
    return IOObject::write(data);
}

void
UDPSocket::asyncWrite(std::vector<char> &data, std::function<void(IOObject &, std::vector<char> &, Error)> callback) {
    IOObject::asyncWrite(data, callback);
}

IOObject::Error UDPSocket::bind(const std::string &ip, int port) {
    return IOObject::bind(ip, port);
}

IOObject::Error UDPSocket::close() {
    return IOObject::close();
}

void UDPSocket::asyncSendTo(const std::string &ip, int port, std::vector<char> &data,
                            std::function<void(IOObject &, std::vector<char> &, Error)> callback) {
    IOObject::asyncSendTo(ip, port, data, callback);
}

IOObject::Error UDPSocket::sendTo(const std::string &ip, int port, std::vector<char> &data) {
    return IOObject::sendTo(ip, port, data);
}

std::pair<Address, IOObject::Error> UDPSocket::recvFrom(std::vector<char> &data) {
    return IOObject::recvFrom(data);
}

void UDPSocket::asyncRecvFrom(std::vector<char> &data,
                              std::function<void(IOObject &, Address, std::vector<char> &, Error)> callback) {
    IOObject::asyncRecvFrom(data, callback);
}

