#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

// C++
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream> // for ifstream

// C
#include <sys/socket.h>

// Custom
#include "Server.hpp"

#define BUFFER_SIZE 1024

class Server;

class HttpHandler {

private:

    int _fd;
    // std::unordered_map<std::string, std::string> _context;

    std::string _req;
    std::string _res;

    // start line
    std::string _method;
    std::string _uri;
    std::string _version;

    // header
    std::string _accept;
    std::string _connection;
    std::string _host;

    // body
    std::string _body;

    // location and file
    std::string _location_path;
    std::string _location_value;
    std::string _root;
    std::string _file;

    int header_only;
    int header_sent;

    std::vector<Server> _serverList;

    Server _server;
    int _server_index;

    HttpHandler(void);

public:

    HttpHandler(int fd);
    ~HttpHandler(void);

    // request part
    void handlingRequest(void);
    int check_host(void);

    // response part
    void handlingResponse(void);
    void set_location_value(std::string input);
    void set_file(std::string input);
    std::string get_root(void);

    // getter
    int getfd(void);
    Server& get_server(void);

    // set server
    void set_server(Server &sv);
    void set_server_list(std::vector<Server> &list);

};

#endif
