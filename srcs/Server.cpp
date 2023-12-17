#include "Server.hpp"

Server::Server(void): _ip(std::string("")), _port(-1), _socket(-1), _isDefaultServer(0) {

}

Server::~Server(void) {

}

// =============== Setter ================ //
void Server::set_ip(std::string ip) {
    this->_ip = ip;
}

void Server::set_port(int port) {
    this->_port = port;
}

void Server::add_server_name(std::string name) {
    this->_serverName.push_back(name);
}

void Server::set_socket(int fd) {
    this->_socket = fd;
}

void Server::set_addr(struct sockaddr_in addr) {
    this->_addr = addr;
}

void Server::set_addr_len(socklen_t len) {
    this->_addrLen = len;
}

// =============== Getter ================== //
std::string Server::get_ip(void) const {
    return (this->_ip);
}

int Server::get_port(void) const {
    return (this->_port);
}

std::string Server::get_server_name(int i) {
    return (this->_serverName[i]);
}

int Server::get_socket(void) {
    return (this->_socket);
}

struct sockaddr_in Server::get_addr(void) {
    return (this->_addr);
}

socklen_t Server::get_addr_len(void) {
    return (this->_addrLen);
}

std::string Server::get_root(void) {
    return (this->_root);
}

//
int Server::check_server_name(std::string name) {
    for (int i = 0; i < (int)this->_serverName.size(); i++) {
        if (this->_serverName[i].compare(name) == 0)
            return (i);
    }
    return (-1);
}

// Location block
void Server::set_root(std::string root) {
    this->_root = std::string(root);
}