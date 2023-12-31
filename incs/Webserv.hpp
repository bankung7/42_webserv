#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// C library
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> // addrinfo, getaddrinfo(), freeaddrinfo()
#include <sys/epoll.h> // epoll_create(), epoll_wait(), epoll_ctl()
#include <unistd.h> // close
#include <stdio.h>

// C++ library
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <ctime>

// Custom Library
#include "Server.hpp"
#include "HttpHandler.hpp"
#include "Conf.hpp"
#include "StrUtils.hpp"

// define
#define MAX_EVENTS 1024
#define BACKLOG 100

class Webserv {

private:
    std::set<int> _fd;
    std::vector<Server> _server;
    std::set<int> _port;
    std::map<int, HttpHandler*> _context;
    std::vector<HttpHandler*> _client;

    int _backlog;
    int _epfd;

public:
    Webserv(void);
    ~Webserv(void);

    // setter
    void add_fd(int);
    void add_server(Server);
    void add_port(int);
    void add_context(int, HttpHandler*);

    // getter
    HttpHandler& get_context(int);

    // general    
    void setup(void);

    // Epoll.cpp
    int polling(void);
    void close_connection(HttpHandler*);
    void check_time_out(void);

    // remover
    void remove_context(int);
    void remove_client(int);

};

#endif
