#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

// C++
#include <iostream>
#include <sstream>
#include <vector>
// #include <unordered_map>

// C
#include <sys/socket.h>

#define BUFFER_SIZE 1024

class HttpHandler {

private:

    int _fd;
    // std::unordered_map<std::string, std::string> _context;

    std::string _req;
    std::string _res;

    std::string _uri;
    std::string _method;
    std::string _version;

    int header_only;
    int header_sent;

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
