//
// Created by mprotsyk on 28.04.22.
//

#include "echo_server.h"


EchoServer::EchoServer(AbstractAsyncHandler *handler, const std::string &ip, int port): server(handler) {
    if (Operations::Error::SUCCESS != Operations::bind(server, ip, port)) {
        throw std::runtime_error("Can't bind to the given address");
    }

    if (Operations::Error::SUCCESS != Operations::listen(server, 100)) {
        throw std::runtime_error("Can't listen");
    }

    Operations::asyncAccept(server, [this](IOObject newObject, Operations::Error error) {
        acceptFunction(newObject, error);
    });

}


void EchoServer::readFunction(IOObject& usedObject, std::vector<char>& newData, Operations::Error error) {
    if (error != Operations::Error::SUCCESS) {
        deleteFunction(usedObject);
        return;
    }
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
        Operations::asyncRead(object, buffer, [this](IOObject& usedObject, std::vector<char>& newData, Operations::Error error) {
            readFunction(usedObject, newData, error);
        });
    }

    Operations::asyncAccept(server, [this](IOObject newObject, Operations::Error error) {
        acceptFunction(newObject, error);
    });
}

void EchoServer::deleteFunction(IOObject obj) {
    std::cout << "Disconnecting ... " << std::endl;
    Operations::close(obj);
    buffers.erase(obj.getDescriptor());
    connections.erase(obj.getDescriptor());
    std::cout << "Active users: " << connections.size() << std::endl;

}
