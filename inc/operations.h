//
// Created by mprotsyk on 27.04.22.
//

#ifndef SERVER_OPERATIONS_H
#define SERVER_OPERATIONS_H

#include <unistd.h>

#include "io_object.h"
#include "event.h"

namespace Operations {
    enum class Error {
        SUCCESS, ERROR
    };
    Error read(IOObject& obj, std::vector<char>& data);
    void asyncRead(IOObject& obj, std::vector<char>& data, std::function<void(IOObject&, std::vector<char>&, Error)> callback);
    Error write(IOObject& obj, std::vector<char>& data);
    void asyncWrite(IOObject& obj, std::vector<char>& data, std::function<void(IOObject&, std::vector<char>&, Error)> callback);
    Error bind(IOObject& obj, const std::string& ip, int port);
    Error listen(IOObject& obj, size_t maxPendingConnections);
    Error accept(IOObject& obj, std::vector<char>& data);

    Error close(IOObject& obj);

    void asyncAccept(IOObject& obj, const std::function<void(IOObject, Error)>& callback);
    std::pair<IOObject, Error> accept(IOObject& obj);


};

#endif //SERVER_OPERATIONS_H
