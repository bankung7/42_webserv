#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

// C
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // for system
#include <sys/stat.h> // stat
#include <errno.h> // errno
#include <string.h> // strerror
#include <sys/wait.h> // for wait

// C++
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <limits> // numeric_limit
#include <ctime>

// Custom
#include "Server.hpp"
#include "Code.hpp"

class HttpHandler {

private:

    int _fd;
    int _port;

    int _status; // for progress status

    time_t _timeout; // for client timeout

    // request part
    std::string _req;
    std::string _body; // for requesy body
    std::string _method;
    std::string _url;
    std::string _version;
    std::size_t _reqContentLength; // content-length
    int _readState; // 0 for header, 1 for body
    int _postType; // url or formdata
    std::size_t _maxClientBodySize; // limit body size

    std::map<std::string, std::string> _parameter;

    std::vector<Server> _server;
    int _serverIndex;

    std::string _loc; // location name
    std::string _location; // location block
    std::string _filepath; // for root + url
    struct stat _fileInfo; // file info
    std::string _root; // root

    // directive
    int _isDirectory;
    int _isRedirection;
    int _isAutoIndex;
    int _isIndex;
    size_t _fileSize;
    std::map<int, std::string> _errorCode; // for error code in the location block
    std::string _temp; // for fileupload

    // cgi part
    int _isCGI; // for cgi checking
    std::string _cgiType; // for bash or python3
    std::string _cgipath;
    std::string _cgiResBody; // for cgi write the output to
    std::string _queryString; // for query string

    std::string _res;
    std::ifstream _file;
    int _resStatusCode;
    std::string _resStatusText;
    std::string _resContentType;

    int _tryFileStatus; // to let the server know not to load to file

    HttpHandler(void);
    
public:
    HttpHandler(int);
    ~HttpHandler(void);

    // setter
    void set_status(int);
    void set_server(std::vector<Server>&);
    void set_res_content_type(void);

    // getter
    int get_fd(void) const;
    int get_status(void) const;
    std::string get_connection_type(void);
    std::time_t get_time_out(void) const;

    // process
    void handle_request(void);
    void reading_request(void);
    void setup_header(void);
    void parsing_request(void);


    void handle_response(void);
    void assign_server_block(void);
    void assign_location_block(void);
    void create_response(void);
    void try_file(void);
    void content_builder(void);

    // cgi
    void handle_cgi(void);

    // res handle
    void open_file(void);
    void uploading_task(void);
    void set_res_status(int, std::string);
    void error_page_set(int, std::string);
    void parsing_error_code(std::string);
    std::string create_res_attribute(std::string, std::string);


    // utils
    void remove_white_space(std::string&);
    int string_to_int(std::string);
    std::size_t string_to_size(std::string);
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