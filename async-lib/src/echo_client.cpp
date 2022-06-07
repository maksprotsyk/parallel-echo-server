//
// Created by mprotsyk on 28.04.22.
//

#include "echo_client.h"

EchoClient::EchoClient(AbstractAsyncHandler* handler, const std::string& ip, int port): client(handler) {
    SPDLOG_LOGGER_INFO(
            spdlog::get(LOGGER_NAME),
            "Connecting"
    );
    if (IOObject::Error::SUCCESS != client.connect(ip, port)) {
        throw std::runtime_error("Can't connect");
    }
    SPDLOG_LOGGER_INFO(
            spdlog::get(LOGGER_NAME),
            "Connected to {}:{}", ip, port
    );

}

void EchoClient::run() {
    while (true) {
        std::string data;
        SPDLOG_LOGGER_INFO(
                spdlog::get(LOGGER_NAME),
                "Waiting for input:"
        );
        std::cin >> data;
        buffer = std::vector<char>(data.begin(), data.end());
        if (IOObject::Error::SUCCESS != client.write(buffer)) {
            throw std::runtime_error("Can't write");
        }
        buffer.clear();
        if (IOObject::Error::SUCCESS != client.read(buffer)) {
            throw std::runtime_error("Can't read");
        }
        SPDLOG_LOGGER_INFO(
                spdlog::get(LOGGER_NAME),
                "Got data: {}",  std::string(buffer.begin(), buffer.end())
        );
    }


}


EchoClient::~EchoClient() {
    client.close();
    SPDLOG_LOGGER_INFO(
            spdlog::get(LOGGER_NAME),
            "Disconnected"
    );

}