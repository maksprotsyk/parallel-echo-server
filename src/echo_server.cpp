//
// Created by mprotsyk on 28.04.22.
//

#include "echo_server.h"


EchoServer::EchoServer(AbstractAsyncHandler *handler, const std::string &ip, int port): server(handler) {
    if (Operations::Error::SUCCESS != Operations::bind(server, ip, port)) {
        throw std::runtime_error("Can't bind to the given address");
    }
    SPDLOG_LOGGER_INFO(
            spdlog::get(LOGGER_NAME),
            "Binded to {}:{}", ip, port
    );


    if (Operations::Error::SUCCESS != Operations::listen(server, 100)) {
        throw std::runtime_error("Can't listen");
    }
    SPDLOG_LOGGER_INFO(
            spdlog::get(LOGGER_NAME),
            "Listening..."
    );

    Operations::asyncAccept(server, [this](IOObject newObject, Operations::Error error) {
        acceptFunction(newObject, error);
    });

}


void EchoServer::readFunction(IOObject& usedObject, std::vector<char>& newData, Operations::Error error) {
    if (error != Operations::Error::SUCCESS) {
        deleteFunction(usedObject);
        return;
    }
    SPDLOG_LOGGER_INFO(
            spdlog::get(LOGGER_NAME),
            "Got data: {}",std::string(newData.begin(), newData.end())
    );
    Operations::asyncWrite(usedObject, newData, [this](IOObject& usedObject, std::vector<char>& newData, Operations::Error error) {
        writeFunction(usedObject, newData, error);
    });
}

void EchoServer::writeFunction(IOObject& usedObject, std::vector<char>& newData, Operations::Error error) {
    if (error != Operations::Error::SUCCESS) {
        deleteFunction(usedObject);
        return;
    }
    newData.clear();
    Operations::asyncRead(usedObject, newData, [this](IOObject& usedObject, std::vector<char>& newData, Operations::Error error) {
        readFunction(usedObject, newData, error);
    });
}

void EchoServer::acceptFunction(IOObject newObject, Operations::Error error) {
    if (error == Operations::Error::SUCCESS) {
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
        Operations::asyncRead(object, buffer, [this](IOObject& usedObject, std::vector<char>& newData, Operations::Error error) {
            readFunction(usedObject, newData, error);
        });
    }

    Operations::asyncAccept(server, [this](IOObject newObject, Operations::Error error) {
        acceptFunction(newObject, error);
    });
}

void EchoServer::deleteFunction(IOObject obj) {
    SPDLOG_LOGGER_INFO(
            spdlog::get(LOGGER_NAME),
            "Disconnecting..."
    );
    Operations::close(obj);
    buffers.erase(obj.getDescriptor());
    connections.erase(obj.getDescriptor());
    SPDLOG_LOGGER_INFO(
            spdlog::get(LOGGER_NAME),
            "Active users: {}", connections.size()
    );

}
