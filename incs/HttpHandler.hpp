#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#define BUFFER_SIZE 1024

#define NOTSTART 0
#define READING 1
#define WRITING 2
#define COMPLETED 3

// C
#include <stdio.h>
#include <unistd.h>

// C++
#include <sstream>
#include <iostream>
#include <vector>

// Custom
#include "Server.hpp"

class HttpHandler {

private:
    int _fd;
    int _status;
    int _port;

    std::string _req;
    std::string _method;
    std::string _url;
    std::string _version;
    // std::string _host;
    // std::string _connection;
    std::string _body;

    std::map<std::string, std::string> _parameter;

    std::vector<Server> _server;
    int _serverIndex;

    std::string _location;

    std::string _res;
    std::string _root;
    std::string _filename;

    HttpHandler(void);
    
public:
    HttpHandler(int);
    ~HttpHandler(void);

    // setter
    void set_status(int);
    void set_server(std::vector<Server>&);

    // getter
    int get_fd(void) const;
    int get_status(void) const;

    // process
    void handle_request(void);
    void parsing_request(void);

    void handle_response(void);

    // utils
    void remove_white_space(std::string&);

};

#endif

// GET / HTTP/1.1
// Host: localhost:8080
// User-Agent: Mozilla/5.0 (X11; Linux aarch64; rv:102.0) Gecko/20100101 Firefox/102.0
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8
// Accept-Language: en-US,en;q=0.5
// Accept-Encoding: gzip, deflate, br
// Connection: keep-alive
// Upgrade-Insecure-Requests: 1
// Sec-Fetch-Dest: document
// Sec-Fetch-Mode: navigate
// Sec-Fetch-Site: cross-site