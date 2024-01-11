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
#include <errno.h>

// C++ library
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <ctime>
#include <csignal> // signal

// Custom Library
#include "Server.hpp"
#include "HttpHandler.hpp"
#include "Conf.hpp"
#include "StrUtils.hpp"

#include "Code.hpp" // for define variable

// define color

class Webserv {

private:
    std::set<int> _fd;
    std::vector<Server> _server;
    std::set<int> _port;
    std::map<int, HttpHandler*> _context;
    std::map<int, int> _cgiList;

    int _backlog;
    int _epfd;

    std::stringstream _log; // text for error writing

public:

    Webserv(void);
    Webserv(std::string);
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
    int epoll_add(int, int);
    void epoll_mod(int, int);
    void epoll_del(int);

    void close_connection(int);
    void check_time_out(void);

    // remover
    void remove_context(int);
    

    // error handling
    void clean_socket(void);
    void clean_context(void);

    //utils
    // void printserverconf(Conf &);

};

#endif
