#include "HttpHandler.hpp"

// ignore default constructor
HttpHandler::HttpHandler(void) {}


HttpHandler::HttpHandler(int fd): _fd(fd) {

}

HttpHandler::~HttpHandler(void) {

}

// https://nginx.org/en/docs/http/request_processing.html
// https://nginx.org/en/docs/dev/development_guide.html#http_request

// request handling part
void HttpHandler::handlingRequest(void) {

    // 1- readind form fd
    int rd;
    char bf[BUFFER_SIZE];
    std::stringstream ss;

    ss << std::noskipws;

    do {
        rd = recv(this->_fd, bf, BUFFER_SIZE, 0);
        if (rd < 1024)
            bf[rd] = 0;

        // TODO: error handling
        if (rd <= 0)
            break ;

        ss << bf;
    } while (rd > 0);

    // save to req
    this->_req = std::string(ss.str());

    std::cout << this->_req << std::endl;

}

// response handling part
void HttpHandler::handlingResponse(void) {

    // TODO: create response

    // just a simple res
    std::stringstream ss;
    ss << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: text/html\r\n"
        << "Content-Length: " << this->_req.size() << "\r\n\r\n"
        << this->_req;

    std::string res = std::string(ss.str());
    std::vector<char> msg;
    msg.insert(msg.begin(), res.begin(), res.end());

    // loop send
    int totalBytes = res.size();
    int bytes = 0;

    while (bytes < totalBytes) {
        int sentBytes = send(this->_fd, msg.data() + bytes, BUFFER_SIZE, MSG_NOSIGNAL);

        // TODO: error handling
        if (sentBytes <= 0) {
            std::cout << "[ERROR]: sending failed " << this->_fd << std::endl;
            break ;
        }

        bytes += sentBytes;
    }

    std::cout << "[DEUBG]: Total sent " << totalBytes << " to " << this->_fd << std::endl;

}


// getter
int HttpHandler::getfd(void) {
    return (this->_fd);
}