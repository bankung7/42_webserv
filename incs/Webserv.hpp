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

    std::vector<int> _socket;
    std::vector<struct sockaddr_in> _addr;
    std::vector<socklen_t>  _addrLen;

    int _socketSize;
    
    // epoll.hpp
    int _epfd;

public:
    Webserv(void);
    ~Webserv(void);

    void start(void);
    void set_config_name(std::string file);

    int check_listener(int fd);

    // test function
    void setup(void);

    // epoll.cpp
    void create_socket(std::string host, int fd, int size);
    int setnonblock(int fd);

    int polling(void);

};

#endif
