//
// Created by mprotsyk on 28.04.22.
//

#include "echo_server.h"


EchoServer::EchoServer(AbstractAsyncHandler *handler, const std::string &ip, int port): server(handler) {
    if (IOObject::Error::SUCCESS != server.bind(ip, port)) {
        throw std::runtime_error("Can't bind to the given address");
    }
    SPDLOG_LOGGER_INFO(
            spdlog::get(LOGGER_NAME),
            "Binded to {}:{}", ip, port
    );


    if (IOObject::Error::SUCCESS != server.listen(100)) {
        throw std::runtime_error("Can't listen");
    }
    SPDLOG_LOGGER_INFO(
            spdlog::get(LOGGER_NAME),
            "Listening..."
    );

    server.asyncAccept([this](IOObject newObject, IOObject::Error error) {
        acceptFunction(newObject, error);
    });

}


void EchoServer::readFunction(IOObject& usedObject, std::vector<char>& newData, IOObject::Error error) {
    if (error != IOObject::Error::SUCCESS) {
        deleteFunction(usedObject);
        return;
    }
    SPDLOG_LOGGER_INFO(
            spdlog::get(LOGGER_NAME),
            "Got data: {}",std::string(newData.begin(), newData.end())
    );
    usedObject.asyncWrite(newData, [this](IOObject& usedObject, std::vector<char>& newData, IOObject::Error error) {
        writeFunction(usedObject, newData, error);
    });
}

void EchoServer::writeFunction(IOObject& usedObject, std::vector<char>& newData, IOObject::Error error) {
    if (error != IOObject::Error::SUCCESS) {
        deleteFunction(usedObject);
        return;
    }
    newData.clear();
    usedObject.asyncRead(newData, [this](IOObject& usedObject, std::vector<char>& newData, IOObject::Error error) {
        readFunction(usedObject, newData, error);
    });
}

void EchoServer::acceptFunction(IOObject newObject, IOObject::Error error) {
    if (error == IOObject::Error::SUCCESS) {
        connections.emplace(
                newObject.getDescriptor(),
                newObject
        );
        buffers.emplace(
                newObject.getDescriptor(),
                std::vector<char>()
        );
        auto& object = connections.at(newObject.getDescriptor());
        auto& buffer = buffers.at(newObject.getDescriptor());
        SPDLOG_LOGGER_INFO(
                spdlog::get(LOGGER_NAME),
                "Accepted connection, currently online users: {}", connections.size()
        );
        object.asyncRead(buffer, [this](IOObject& usedObject, std::vector<char>& newData, IOObject::Error error) {
            readFunction(usedObject, newData, error);
        });
    }
    server.asyncAccept([this](IOObject newObject, IOObject::Error error) {
        acceptFunction(newObject, error);
    });
}

void EchoServer::deleteFunction(IOObject obj) {
    SPDLOG_LOGGER_INFO(
            spdlog::get(LOGGER_NAME),
            "Disconnecting..."
    );
    obj.close();
    buffers.erase(obj.getDescriptor());
    connections.erase(obj.getDescriptor());
    SPDLOG_LOGGER_INFO(
            spdlog::get(LOGGER_NAME),
            "Active users: {}", connections.size()
    );

}
