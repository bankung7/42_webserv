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
#include <vector>

class Server {

private:
    // int         _duplicated;
    int         _fd;
    std::vector<std::string> _serverName;
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
    void add_server_name(std::string);
    void set_port(int);
    void set_default_error_page(std::string);
    void set_max_client_body_size(size_t);
    void set_root(std::string);
    void set_allowed_method(std::string);
    void set_location(std::string, std::string);

    // getter
    int get_fd(void) const;
    int get_port(void) const;
    std::string get_root(void) const;
    std::string get_server_name(std::string) const;
    std::string get_location(std::string);
    std::string best_match_location(std::string);

    // checker
    int has_server_name(std::string);

    // general function
    void initiated(int);


};

#endif


/// localhost:8080
/// localhost:8080/index.html
/// localhost:8080/sub1

/// localhost:8080/sub1/
/// localhost:8080/sub1/index.html