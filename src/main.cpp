#include <iostream>
#include <thread>

#include "epoll_async_handler.h"
#include "socket_server.h"
#include "operations.h"
#include "io_object.h"

#include "echo_server.h"





int main(int argc, char *argv[]) {
    auto* handler = new EpollAsyncHandler(100);

    //fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    //fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK);
    //AsyncFile inFile(handler, STDIN_FILENO);
    //AsyncFile outFile(handler, STDOUT_FILENO);

    //inFile.scheduleRead([&inFile, &outFile](const std::shared_ptr<Bytes>& bytes, AsyncFile::OperationResult op) {
    //    readFunction(inFile, bytes, op);
    //    std::cout << (int)op << std::endl;

    //    outFile.scheduleWrite(bytes, [](AsyncFile::OperationResult op){std::cout << "op" << std::endl;});
    //});


    EchoServer server(handler, "127.0.0.1", 8080);

    handler->runEventLoop();

    delete handler;



   return 0;
}