#ifndef SERVER_HPP
#define SERVER_HPP

// C
#include <unistd.h> // close()
#include <string.h> // memset()
#include <fcntl.h> // fcntl()

// C server
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h> // socket(), bind(), listen()
#include <netdb.h> // addrinfo, getaddrinfo(), freeaddrinfo()
#include <sys/epoll.h> // epoll_create(), epoll_wait(), epoll_ctl()

// C++
#include <iostream>

// Own hpp
#include "HttpHandler.hpp"

// Parameter
#define PORT "8080"
#define BACKLOG 25
#define MAX_EVENTS 25
#define BUFFER_SIZE 1024

class Server {

private:
    int _listener;
    int _epfd;

public:
    Server(void);
    ~Server(void);

    int get_listener(void);
    int polling(void);
    int setnonblock(int fd);

};

#endif

// Server.cpp
// Running the server, epoll
// push the task of request and response dealing to HttpHandler.cpp
// CGI?

// still does not handle when it unexpected closed