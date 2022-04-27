//
// Created by mprotsyk on 27.04.22.
//

#include "io_object.h"

int IOObject::getDescriptor() const {
    return fd;
}

IOObject::IOObject(AbstractAsyncHandler *handler_, int fd_): handler(handler_), fd(fd_) {

}

AbstractAsyncHandler *IOObject::getHandler() const {
    return handler;
}
