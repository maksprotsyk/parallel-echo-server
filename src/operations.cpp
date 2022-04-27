//
// Created by mprotsyk on 27.04.22.
//

#include "operations.h"

#include <utility>

namespace Operations {
    constexpr int READ_BUFFER_SIZE = 512;

    namespace {
        std::function<void()> readCallback(IOObject& obj, std::vector<char>& data, std::function<void(IOObject&, std::vector<char>&, Error)> callback) {
            return [fd = obj.getDescriptor(), &obj, &data, callback](
            ) {
                char buffer[READ_BUFFER_SIZE];
                while (true) {
                    auto res = ::read(fd, buffer, READ_BUFFER_SIZE);
                    if (res < 0) {
                        if (errno == EWOULDBLOCK || errno == EAGAIN) {
                            callback(obj, data, Error::SUCCESS);
                            return;
                        }
                        if (errno != EINTR) {
                            callback(obj, data, Error::ERROR);
                            return;
                        }
                        continue;
                    }
                    if (res == 0) {
                        callback(obj, data, Error::ERROR);
                        return;
                    }
                    data.insert(data.end(), buffer, buffer + res);

                }
            };
        }

        std::function<void()> writeCallback(IOObject& obj, std::vector<char>& data, std::function<void(IOObject&, std::vector<char>&, Error)> callback) {
            return [fd = obj.getDescriptor(), &data, callback, &obj](
            ) {
                auto res = ::write(fd, data.data(), data.size());
                if (res != data.size()) {
                    callback(obj, data, Error::ERROR);
                } else {
                    callback(obj, data, Error::SUCCESS);
                }
            };
        }

        std::function<void()> acceptCallback(IOObject& obj,  std::function<void(IOObject, Error)> callback) {
            return [&obj, callback] () {
                int newFd = ::accept4(obj.getDescriptor(), nullptr, nullptr, SOCK_NONBLOCK);
                if (newFd < 0) {
                    callback(IOObject(obj.getHandler(), newFd), Error::ERROR);
                    return;
                }
                callback(IOObject(obj.getHandler(), newFd), Error::SUCCESS);

            };
        }

    }


    Error read(IOObject& obj, std::vector<char>& data) {
        Error error;
        Event event(obj.getHandler(), obj.getDescriptor(), AbstractEvent::Type::READY_IN);
        event.setCallback(readCallback(obj, data,  [&error](IOObject&, std::vector<char>&, Error res) {
            error = res;
        }));
        event.schedule();
        event.wait();
        return error;
    }
    void asyncRead(IOObject& obj, std::vector<char>& data, std::function<void(IOObject&, std::vector<char>&, Error)> callback) {
        Event event(obj.getHandler(), obj.getDescriptor(), AbstractEvent::Type::READY_IN);
        event.setCallback(readCallback(obj, data,  std::move(callback)));
        event.schedule();
        event.detach();
    }

    Error write(IOObject& obj, std::vector<char>& data) {
        Error error;
        Event event(obj.getHandler(), obj.getDescriptor(), AbstractEvent::Type::READY_OUT);
        event.setCallback(writeCallback(obj, data,  [&error](IOObject&, std::vector<char>&, Error res) {
            error = res;
        }));
        event.schedule();
        event.wait();
        return error;
    }
    void asyncWrite(IOObject& obj, std::vector<char>& data, std::function<void(IOObject&, std::vector<char>&, Error)> callback) {
        Event event(obj.getHandler(), obj.getDescriptor(), AbstractEvent::Type::READY_OUT);
        event.setCallback(writeCallback(obj, data,  std::move(callback)));
        event.schedule();
        event.detach();
    }


    Error bind(IOObject& obj, const std::string& ip, int port) {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(ip.c_str());
        address.sin_port = htons(port);
        int res = ::bind(obj.getDescriptor(), (struct sockaddr *)&address, sizeof(address));
        if (res < 0) {
            return Error::ERROR;
        }
        return Error::SUCCESS;

    }

    Error listen(IOObject& obj, size_t maxPendingConnections) {
        int res = ::listen(obj.getDescriptor(), static_cast<int>(maxPendingConnections));
        if (res < 0) {
            return Error::ERROR;
        }
        return Error::SUCCESS;
    }

    void asyncAccept(IOObject& obj, const std::function<void(IOObject, Error)>& callback) {
        Event event(obj.getHandler(), obj.getDescriptor(), AbstractEvent::Type::READY_IN);
        event.setCallback(acceptCallback(obj, callback));
        event.schedule();
        event.detach();
    }

    std::pair<IOObject, Error> accept(IOObject& obj) {
        IOObject newObj(nullptr, -1);
        Error error;
        Event event(obj.getHandler(), obj.getDescriptor(), AbstractEvent::Type::READY_IN);
        event.setCallback(acceptCallback(obj, [&newObj, &error](IOObject accepted, Error res){
            error = res;
            newObj = accepted;
        }));
        event.schedule();
        event.wait();
        return {newObj, error};
    }

    Error close(IOObject& obj) {
        auto res = ::close(obj.getDescriptor());
        if (res < 0) {
            return Error::ERROR;
        }
        return Error::SUCCESS;
    }

}