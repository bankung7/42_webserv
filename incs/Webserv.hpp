#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// C library
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h> // socket(), bind(), listen()
#include <netdb.h> // addrinfo, getaddrinfo(), freeaddrinfo()
#include <sys/epoll.h> // epoll_create(), epoll_wait(), epoll_ctl()
#include <unistd.h> // close
#include <fcntl.h> // O_NONBLOCK

// C++ library
#include <iostream>
#include <map>

// Custom hpp
#include "HttpHandler.hpp"
#include "Server.hpp"

// define
#define BACKLOG 25
#define MAX_EVENTS 25
#define BUFFER_SIZE 1024

class Webserv : public Server {

private:

    std::string _configFile;

    std::vector<Server> _server;
    int _serverSize;

    std::vector<int> _socketList;
    
    // epoll.hpp
    int _epfd;

public:
    Webserv(void);
    ~Webserv(void);

    // Configuration Part
    void set_config_name(std::string file);

    void start(void);

    void create_socket(std::vector<Server> &server);
    int setnonblock(int fd);
    int check_listener(int fd);

    // setter
    void add_socket(int fd);

    // getter
    Server& get_server(int i);

    // epoll.cpp
    int polling(void);
    void add_listener_to_epoll(void);

    // Test function
    void setup(void);

};

#endif
