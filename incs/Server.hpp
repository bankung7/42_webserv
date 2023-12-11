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
    std::string _host;
    std::vector<int> _port;
    int _size;

    // TODO: multiple server name
    std::vector<std::string> _serverName;

public:
    Server(void);
    ~Server(void);

    void add_host(std::string host);
    void add_port(int port);

    std::string get_host(void) const;
    int get_port(int i) const;
    std::vector<int>& get_all_port(void);
    int get_size(void) const;
    std::string get_server_name(int i);

    // test funciton
    void set_size(int n);
    void add_server_name(std::string name);

};

#endif

// all server setting is stored here
// plan to user the std::map
// getter the value for each attribute