#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// C library
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> // addrinfo, getaddrinfo(), freeaddrinfo()
#include <sys/epoll.h> // epoll_create(), epoll_wait(), epoll_ctl()
#include <unistd.h> // close

// C++ library
#include <iostream>
#include <vector>
#include <set>

// Custom Library
#include "Server.hpp"

// define
#define MAX_EVENTS 25
#define BACKLOG 20

class Webserv {

private:
    std::set<int> _fd;
    std::vector<Server> _server;
    std::set<int> _port;

    int _backlog;
    int _epfd;

public:
    Webserv(void);
    ~Webserv(void);

    // setter
    void add_fd(int);
    void add_server(Server);
    void add_port(int);

    // getter

    // general    
    void setup(void);

    // Epoll.cpp
    int polling(void);

};

#endif
