#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <map>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h> // socket(), bind(), listen()
#include <netdb.h> // addrinfo, getaddrinfo(), freeaddrinfo()
#include <string.h> // memset

class Server {

private:
    std::string _ip; // for specific ip
    int _port; // only 1 port per server block

    // TODO: server_name
    std::vector<std::string> _serverName;

    // server's socket info
    int _socket;
    struct sockaddr_in _addr;
    socklen_t _addrLen;

    // default server or not
    int _isDefaultServer;

    // for location block
    std::map<std::string, std::string> _location;
    std::string _root; // main root, throw error if not found.


public:
    Server(void);
    ~Server(void);

    // setter
    void set_ip(std::string ip);
    void set_port(int port);
    void add_server_name(std::string name);
    void set_socket(int fd);
    void set_addr(struct sockaddr_in addr);
    void set_addr_len(socklen_t len);

    // getter
    std::string get_ip(void) const;
    int get_port(void) const;
    std::string get_server_name(int i);
    int get_socket(void);
    struct sockaddr_in get_addr(void);
    socklen_t get_addr_len(void);
    std::string get_root(void);

    int check_server_name(std::string name);

    // Location block
    void add_location(std::string loc, std::string value);
    // std::string get_location(std::string loc);
    std::map<std::string, std::string> get_location_block(void);
    void set_root(std::string root);

};

#endif

// all server setting is stored here
// plan to user the std::map
// getter the value for each attribute