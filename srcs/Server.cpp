#include "Server.hpp"

Server::Server(void) {

}

Server::~Server(void) {

}

// setter
void Server::set_fd(int fd) {
    this->_fd = fd;
};

void Server::add_server_name(std::string name) {
    this->_serverName.push_back(std::string(name));
};

void Server::set_port(int port) {
    this->_port = port;
};

void Server::set_default_error_page(std::string path) {
    this->_defaultErrorPage = std::string(path);
};

void Server::set_max_client_body_size(size_t size) {
    this->_maxClientBodySize = size;
};

void Server::set_root(std::string root) {
    this->_root = std::string(root);
};

void Server::set_allowed_method(std::string method) {
    this->_allowedMethod = std::string(method);
};

void Server::set_location(std::string loc, std::string value) {
    this->_location.insert(std::pair<std::string, std::string>(loc, value));
};

// getter
int Server::get_fd(void) const {
    return (this->_fd);
}

int Server::get_port(void) const {
    return (this->_port);
}

std::string Server::get_root(void) const {
    return (std::string(this->_root));
}

std::string Server::get_server_name(std::string name) const{
    for (int i = 0; i < (int)this->_serverName.size(); i++) {
        if (this->_serverName[i].compare(name) == 0)
            return (std::string(this->_serverName[i]));
    }
    return (std::string(""));
}

std::string Server::get_location(std::string loc) {

    if (this->_location.find(loc) != this->_location.end())
        return (std::string(this->_location[loc]));
    return (std::string(""));
}

// checker
int Server::has_server_name(std::string name) {
    for (int i = 0; i < (int)this->_serverName.size(); i++) {
        if (this->_serverName[i].compare(name) == 0) {
            return (1);
        }
    }
    return (0);
}

std::string Server::best_match_location(std::string loc) {

    std::map<std::string, std::string>::iterator it = this->_location.begin();

    int mostMatch = 0;
    std::string output;
    // std::cout << "total size of location: " << this->_location.size() << std::endl;
    for (; it != this->_location.end(); it++) {
        int size = it->first.size();
        // std::cout << it->first << std::endl;
        if (loc.compare(0, it->first.size(), it->first) == 0 && size >= mostMatch) {
            // std::cout << "most match found => " << it->first << std::endl;
            mostMatch = size;
            output = std::string(it->first);
        }
    }

    return (std::string(output));
}

// general
void Server::initiated(int backlog) {

    // create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        throw std::runtime_error("[ERROR]: create socket failed");
    
    // set socket for reusing address
    int optval = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)
        throw std::runtime_error("[ERROR]: set socket option failed");
    
    // struct sockaddr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;                  // for IPv4
    addr.sin_port = htons(this->get_port());    // set to specific port
    addr.sin_addr.s_addr = INADDR_ANY;          // for all IP address

    // bind
    if (bind(fd, (struct sockaddr*)&addr, (int)(sizeof(addr))) == -1)
        throw std::runtime_error("[ERROR]: bind port failed");

    // set nonblocking
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("[ERROR]: set non-blocking failed");
    
    // listen
    if (listen(fd, backlog) == -1)
        throw std::runtime_error("[ERROR]: listen failed");

    // set socket to it fd
    this->_fd = fd;
    
};