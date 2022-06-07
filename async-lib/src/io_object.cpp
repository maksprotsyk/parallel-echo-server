//
// Created by mprotsyk on 27.04.22.
//

#include "io_object.h"

constexpr int READ_BUFFER_SIZE = 512;

std::function<void()> IOObject::readCallback(
        std::vector<char>& data,
        std::function<void(IOObject&, std::vector<char>&, Error)> callback) {
    return [fd = getDescriptor(), this, &data, callback](
    ) {
        char buffer[READ_BUFFER_SIZE];
        while (true) {
            auto res = ::read(fd, buffer, READ_BUFFER_SIZE);
            if (res < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    callback(*this, data, Error::SUCCESS);
                    return;
                }
                if (errno != EINTR) {
                    callback(*this, data, Error::ERROR);
                    return;
                }
                continue;
            }
            if (res == 0) {
                callback(*this, data, Error::ERROR);
                return;
            }
            data.insert(data.end(), buffer, buffer + res);

        }
    };
}

std::function<void()> IOObject::writeCallback(
        std::vector<char>& data,
        std::function<void(IOObject&, std::vector<char>&, Error)> callback) {
    return [fd = getDescriptor(), &data, callback, this](
    ) {
        auto res = ::write(fd, data.data(), data.size());
        if (res != data.size()) {
            callback(*this, data, Error::ERROR);
        } else {
            callback(*this, data, Error::SUCCESS);
        }
    };
}


std::function<void()> IOObject::acceptCallback(std::function<void(IOObject, Error)> callback) {
    return [this, callback] () {
        int newFd = ::accept4(getDescriptor(), nullptr, nullptr, SOCK_NONBLOCK);
        if (newFd < 0) {
            callback(IOObject(getHandler(), newFd), Error::ERROR);
            return;
        }
        callback(IOObject(getHandler(), newFd), Error::SUCCESS);

    };
}

IOObject::Error IOObject::read(std::vector<char>& data) {
    Error error;
    Event event(getHandler(), getDescriptor(), Event::Type::READY_IN);
    event.setCallback(readCallback(data,  [&error](IOObject&, std::vector<char>&, Error res) {
        error = res;
    }));
    event.schedule();
    event.wait();
    return error;
}
void IOObject::asyncRead(
        std::vector<char>& data,
        std::function<void(IOObject&, std::vector<char>&, Error)> callback
        ) {
    Event event(getHandler(), getDescriptor(), Event::Type::READY_IN);
    event.setCallback(readCallback(data,  std::move(callback)));
    event.schedule();
    event.detach();
}

IOObject::Error IOObject::write(std::vector<char>& data) {
    Error error;
    Event event(getHandler(), getDescriptor(), Event::Type::READY_OUT);
    event.setCallback(writeCallback(data,  [&error](IOObject&, std::vector<char>&, Error res) {
        error = res;
    }));
    event.schedule();
    event.wait();
    return error;
}
void IOObject::asyncWrite(std::vector<char>& data, std::function<void(IOObject&, std::vector<char>&, Error)> callback) {
    Event event(getHandler(), getDescriptor(), Event::Type::READY_OUT);
    event.setCallback(writeCallback(data,  std::move(callback)));
    event.schedule();
    event.detach();
}


IOObject::Error IOObject::bind(const std::string& ip, int port) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip.c_str());
    address.sin_port = htons(port);
    int res = ::bind(getDescriptor(), (struct sockaddr *)&address, sizeof(address));
    if (res < 0) {
        return Error::ERROR;
    }
    return Error::SUCCESS;

}

IOObject::Error IOObject::listen(size_t maxPendingConnections) {
    int res = ::listen(getDescriptor(), static_cast<int>(maxPendingConnections));
    if (res < 0) {
        return Error::ERROR;
    }
    return Error::SUCCESS;
}

void IOObject::asyncAccept(const std::function<void(IOObject, Error)>& callback) {
    Event event(getHandler(), getDescriptor(), Event::Type::READY_IN);
    event.setCallback(acceptCallback(callback));
    event.schedule();
    event.detach();
}

std::pair<IOObject, IOObject::Error> IOObject::accept() {
    IOObject newObj(nullptr, -1);
    Error error;
    Event event(getHandler(), getDescriptor(), Event::Type::READY_IN);
    event.setCallback(acceptCallback([&newObj, &error](IOObject accepted, Error res){
        error = res;
        newObj = accepted;
    }));
    event.schedule();
    event.wait();
    return {newObj, error};
}

IOObject::Error IOObject::close() {
    auto res = ::close(getDescriptor());
    if (res < 0) {
        return Error::ERROR;
    }
    return Error::SUCCESS;
}


IOObject::Error IOObject::connect(const std::string& ip, int port) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip.c_str());
    address.sin_port = htons(port);
    int res = ::connect(getDescriptor(), (struct sockaddr *)&address, sizeof(address));
    if (res < 0 && errno != EINPROGRESS) {
        return Error::ERROR;
    }
    if (res == 0) {
        return Error::SUCCESS;
    }
    Event event(getHandler(), getDescriptor(), Event::Type::READY_OUT);
    event.setCallback([](){});
    event.schedule();
    event.wait();
    return Error::SUCCESS;
}

void IOObject::asyncConnect(const std::string& ip, int port, std::function<void(IOObject&, Error)> callback) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip.c_str());
    address.sin_port = htons(port);
    int res = ::connect(getDescriptor(), (struct sockaddr *)&address, sizeof(address));
    if (res < 0 && errno != EINPROGRESS) {
        callback(*this, Error::ERROR);
        return;
    }
    if (res == 0) {
        callback(*this, Error::SUCCESS);
        return;
    }

    Event event(getHandler(), getDescriptor(), Event::Type::READY_OUT);
    event.setCallback([this, callback](){ callback(*this, Error::SUCCESS);});
    event.schedule();
    event.detach();
}

int IOObject::getDescriptor() const {
    return fd;
}

IOObject::IOObject(AbstractAsyncHandler *handler_, int fd_): handler(handler_), fd(fd_) {
    if (fd < 0) {
        throw std::runtime_error("Invalid fd");
    }
}

AbstractAsyncHandler *IOObject::getHandler() const {
    return handler;
}

std::function<void()> IOObject::sendToCallback(
        const std::string &ip,
        int port,
        std::vector<char> &data,
        std::function<void(IOObject &, std::vector<char> &, Error)> callback
        ) {
    return [fd = getDescriptor(), &data, callback, this, ip, port](
    ) {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(ip.c_str());
        address.sin_port = htons(port);

        auto res =  sendto(
                fd, data.data(), data.size(),
                MSG_CONFIRM,
                (const struct sockaddr *) &address,
                        sizeof(address));
        if (res != data.size()) {
            callback(*this, data, Error::ERROR);
        } else {
            callback(*this, data, Error::SUCCESS);
        }
    };
}

void
IOObject::asyncSendTo(
        const std::string& ip,
        int port,
        std::vector<char> &data,
        std::function<void(IOObject &, std::vector<char> &, Error)> callback
        ) {
    Event event(getHandler(), getDescriptor(), Event::Type::READY_OUT);
    event.setCallback(sendToCallback(ip, port, data,  std::move(callback)));
    event.schedule();
    event.detach();
}

IOObject::Error IOObject::sendTo(const std::string &ip, int port, std::vector<char> &data) {
    Error error;
    Event event(getHandler(), getDescriptor(), Event::Type::READY_OUT);
    event.setCallback(sendToCallback(
            ip,
            port,
            data,
            [&error](IOObject&, std::vector<char>&, Error res
            ) {
        error = res;
    }));
    event.schedule();
    event.wait();
    return error;
}


std::pair<Address, IOObject::Error> IOObject::recvFrom(std::vector<char> &data) {
    Error error;
    Address address;
    Event event(getHandler(), getDescriptor(), Event::Type::READY_IN);
    event.setCallback(recvFromCallback(
            data,
            [&error, &address](IOObject&, Address sender, std::vector<char>&, Error res) {
                error = res;
                address = std::move(sender);
            }));
    event.schedule();
    event.wait();
    return std::make_pair(address, error);
}

void IOObject::asyncRecvFrom(
        std::vector<char> &data,
        std::function<void(IOObject &, Address, std::vector<char> &, Error)> callback) {
    Event event(getHandler(), getDescriptor(), Event::Type::READY_IN);
    event.setCallback(recvFromCallback(data,  std::move(callback)));
    event.schedule();
    event.detach();

}

std::function<void()> IOObject::recvFromCallback(
        std::vector<char> &data,
        std::function<void(IOObject &, Address, std::vector<char> &, Error)> callback
        ) {
    return [fd = getDescriptor(), this, &data, callback](
    ) {
        sockaddr_in address{};
        socklen_t len;
        char buffer[READ_BUFFER_SIZE];
        while (true) {
            auto res = ::recvfrom(
                    fd,  buffer, READ_BUFFER_SIZE,
                    MSG_PEEK, (struct sockaddr *) &address, &len
            );
            if (res > 0) {
                char* ip_c = inet_ntoa(address.sin_addr);
                auto port = ntohs(address.sin_port );
                Address sender;
                sender.ip = ip_c;
                sender.port = port;
                callback(*this, sender,  data, Error::SUCCESS);
                data.clear();
            }
            if (res < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    return;
                }
                if (errno != EINTR) {
                    callback(*this, Address{},  data, Error::ERROR);
                    return;
                }
                continue;
            }
            if (res == 0) {
                return;
            }
            data.insert(data.end(), buffer, buffer + res);

        }
    };
}
