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
void Server::add_location(std::string loc, std::string value) {
    if (this->_location.find(loc) != this->_location.end())
        throw std::runtime_error("[ERROR]: Location path duplicated");
    this->_location.insert(std::pair<std::string, std::string>(loc, value));
}

std::string Server::get_location(std::string loc) {

    // std::cout << "[" << loc << "]" << std::endl;

    // try the exact match, if found return
    if (this->_location.find(loc) != this->_location.end()) {
        std::cout << "found exact match " << loc << std::endl;
        return (this->_location[loc]);
    }

    // try most match by block to block
    std::map<std::string, std::string>::iterator it = this->_location.begin();
    std::map<std::string, std::string>::iterator output;
    int match_value = 0;
    for (; it != this->_location.end(); it++) {

        // std::cout << "matching " << it->first << " with " << loc << std::endl;

        // if input (loc) length longer than value, just continue;
        if ((int)loc.size() < (int)it->first.size()) {
            std::cout << loc << " is shorter than " << it->first << std::endl;
            continue;
        }

        int match = -1;
        for (int i = 0; i < (int)it->first.size(); i++, match++) {
            if (loc[i] != it->first[i]) {
                // std::cout << i << " " << match << " " << loc[match] << " => " << it->first[match] << std::endl;
                break ;
            }
        }

        // std::cout << match << " " << it->first[match] << std::endl;

        if (it->first[match] == '/' && match > match_value) {
            match_value = match;
            output = it;
        }

    }

    std::cout << "[DEBUG]: found most match [" << match_value << "] long is [" << output->first << "]" << std::endl;

    // if match better than "/"
    if (match_value > 0)
        return (output->second);

    return (this->_location["/"]);
}


void Server::set_root(std::string root) {
    this->_root = std::string(root);
}