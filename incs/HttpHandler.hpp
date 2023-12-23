#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

// C

// C++
#include <iostream>

// Custom
#include "Server.hpp"

class HttpHandler {

private:
    int _fd;

    std::string _req;
    std::string _method;
    std::string _url;
    std::string _version;
    std::string _host;
    std::string _connection;
    std::string _body;

    Server _server;
    std::string _location;

    std::string _res;
    std::string _root;
    std::string _filename;

    
public:
    HttpHandler(void);
    HttpHandler(int);
    ~HttpHandler(void);

    // setter


    // getter
    int get_fd(void) const;


};

#endif
