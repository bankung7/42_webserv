#ifndef SERVER_HPP
#define SERVER_HPP

// C library
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h> // socket(), bind(), listen()
#include <netdb.h> // addrinfo, getaddrinfo(), freeaddrinfo()
#include <string.h> // memset, strerror
#include <fcntl.h> // O_NONBLOCK
#include <errno.h> // errno

// C++ library
#include <iostream>
#include <map>
#include <vector>

// Customs
#include "StrUtils.hpp"

class Server {

private:

    int         _fd;
    std::vector<std::string> _serverName;
    int         _port;

    std::map<int, std::string> _errorCode;
    size_t      _maxClientBodySize;

    std::string _root; // mandatory
    std::string _allowedMethod; // mandatory

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
    int is_server_name_defined(void) const;
    std::string get_location(std::string);
    std::string best_match_location(std::string);
    std::string get_error_code(int);

    // checker
    int has_server_name(std::string);

    // general function
    void initiated(int);
    void remove_white_space(std::string &);
    int string_to_int(std::string str);

};

#endif
