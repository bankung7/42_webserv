#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#define BUFFER_SIZE 1024

#define NOTSTART 0
#define READING 1
#define WRITING 2
#define COMPLETED 3
#define CLOSED 9

#define POST_READ_PHASE 1
#define FIND_CONFIG_PHASE 2
#define REWRITE_PHASE 3
#define TRY_FILE_PHASE 4
#define CONTENT_PHASE 5
#define COMPLETE_PHASE 6

// C
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // for system

// C++
#include <sstream>
#include <iostream>
#include <fstream>
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
    std::string _reqContentType;
    std::string _body;

    std::map<std::string, std::string> _parameter;

    std::vector<Server> _server;
    int _serverIndex;

    std::string _loc; // location name
    std::string _location; // location block
    std::string _path;
    std::string _filename;
    std::string _filepath; // for root + url
    std::string _root; // root
    std::string _droot; // root directive from server block
    int _isDirectory;
    int _isRedirection;
    int _isAutoIndex;
    int _isIndex;
    int _isCGI;

    std::string _res;
    std::ifstream _file;
    int _resStatusCode;
    std::string _resStatusText;
    std::string _resContentType;
    int _tryFileStatus;

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
    void assign_server_block(void);
    void assign_location_block(void);
    void create_response(void);
    void try_file(void);
    void content_builder(void);

    // res handle
    void set_res_status(int, std::string);
    void error_page_set(int, std::string);

    // utils
    void remove_white_space(std::string&);
    int string_to_int(std::string);
    std::string int_to_string(int);

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

// weird case, can't reproduce
// [DEBUG]: New connection to 6
// [DEBUG]: Set 6 to EPOLLIN state
// [DEBUG]: reading state
// [ERROR]: The connection was closed or error occured
// [DEBUG]: reading incoming 0
// basic_string::erase: __pos (which is 18446744073709551615) > this->size() (which is 0)


// Black: 30
// Red: 31
// Green: 32
// Yellow: 33
// Blue: 34
// Magenta: 35
// Cyan: 36
// White: 37
// Reset: 0