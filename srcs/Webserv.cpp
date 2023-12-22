#include "Webserv.hpp"

Webserv::Webserv(void): _backlog(20) {

    std::cout << "[DEBUG]: Webserv initiated" << std::endl;

    // Manually create the server block
    Server sv1, sv2, sv3;
    sv1.set_server_name("webserv1");
    sv1.set_port(8080);
    sv1.set_location("/", "allowedMethod:GET;return:;root:/sites/www1;directoryListing:;index:index.html;");
    add_server(sv1);

    sv2.set_server_name("webserv2");
    sv2.set_port(8080);
    sv2.set_location("/", "allowedMethod:GET;return:;root:/sites/www2;directoryListing:;index:index.html;");
    add_server(sv2);

    sv3.set_server_name("webserv3");
    sv3.set_port(8081);
    sv3.set_location("/", "allowedMethod:GET;return:;root:/sites/www3;directoryListing:;index:index.html;");
    add_server(sv3);

    // setup the server and initiated
    try {
        setup();
        polling();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

Webserv::~Webserv(void) {

}

// Configuration Parsing Part

/// == Test Function == ///


// Server Structure Setup Part
void Webserv::setup(void) {

    std::cout << "[DEBUG]: Setting up the server" << std::endl;
    
    // loop read the _server to initiated
    for (int i = 0; i < (int)this->_server.size(); i++) {
        int port = this->_server[i].get_port();
        if (this->_port.find(port) == this->_port.end()) {
            this->_server[i].initiated(BACKLOG);
            this->add_fd(this->_server[i].get_fd());
            this->add_port(port);
            std::cout << "[DEBUG]: Create socket for port " << port << std::endl;
            continue ;
        }
        std::cout << "[WARNING]: Port was duplicated " << port << std::endl;
    }
};

// Server Initialization Part

// EPOLL part => Epoll.cpp

// setter
void Webserv::add_fd(int fd) {
    this->_fd.insert(fd);
}

void Webserv::add_server(Server sv) {
    this->_server.push_back(sv);
}

void Webserv::add_port(int port) {
    this->_port.insert(port);
}

// getter
