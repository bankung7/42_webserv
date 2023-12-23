#include "HttpHandler.hpp"

HttpHandler::HttpHandler(void) {

}

HttpHandler::HttpHandler(int fd): _fd(fd) {

}

HttpHandler::~HttpHandler(void) {

}

// setter

// getter
int HttpHandler::get_fd(void) const {
    return (this->_fd);
}