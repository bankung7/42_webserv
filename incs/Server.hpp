#ifndef SERVER_HPP
#define SERVER_HPP

// C library
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h> // socket(), bind(), listen()
#include <netdb.h> // addrinfo, getaddrinfo(), freeaddrinfo()
#include <string.h> // memset
#include <fcntl.h> // O_NONBLOCK

// C++ library
#include <iostream>
#include <map>

class Server {

private:
    int         _fd;
    std::string _serverName;
    int         _port;
    std::string _defaultErrorPage;
    size_t      _maxClientBodySize;
    std::string _root;
    std::string _allowedMethod;
    std::map<std::string, std::string> _location;

public:
    Server(void);
    ~Server(void);

    // setter
    void set_fd(int);
    void set_server_name(std::string);
    void set_port(int);
    void set_default_error_page(std::string);
    void set_max_client_body_size(size_t);
    void set_root(std::string);
    void set_allowed_method(std::string);
    void set_location(std::string, std::string);

    // getter
    int get_fd(void) const;
    int get_port(void) const;

    // general function
    void initiated(int);


};

#endif
