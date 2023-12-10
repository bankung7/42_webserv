#include "Server.hpp"

Server::Server(void) {
    this->_size = 0;
}

Server::~Server(void) {

}

// add host
void Server::add_host(std::string host) {
    this->_host = host;
}

// add port
void Server::add_port(int port) {
    this->_port.push_back(port);
    this->_size++;
}

// getter
std::string Server::get_host(void) const {
    if (this->_host.size() == 0)
        return (std::string(""));
    return (this->_host);
}

int Server::get_port(int i) const {
    if (i >= this->_size)
        return (-1);
    return (this->_port[i]);
}

int Server::get_size(void) const {
    return (this->_size);
}

std::string Server::get_server_name(void) const {
    return (this->_serverName);
}

// test function
void Server::set_size(int n) {
    this->_size = n;
}

void Server::set_server_name(std::string name) {
    this->_serverName = std::string(name);
}