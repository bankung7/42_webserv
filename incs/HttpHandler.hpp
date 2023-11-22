#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

// C++
#include <iostream>
#include <sstream>
#include <vector>

// C
#include <sys/socket.h>

#define BUFFER_SIZE 1024

class HttpHandler {

private:

    int _fd;
    std::string _req;
    std::string _res;

    HttpHandler(void);

public:

    HttpHandler(int fd);
    ~HttpHandler(void);

    // request part
    void handlingRequest(void);

    // response part
    void handlingResponse(void);

    // getter
    int getfd(void);

};

#endif
