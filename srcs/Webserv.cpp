#include "Webserv.hpp"

Webserv::Webserv(void): _backlog(20) {

    std::cout << "[DEBUG]: Webserv initiated" << std::endl;

    try {
		Conf cf;
		cf.parseconf(this->_server);
        // setup();
        // polling();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

Webserv::Webserv(std::string filename){
    Conf cf(filename);
}

Webserv::~Webserv(void) {

}

// Configuration Parsing Part

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
            std::cout << "[DEBUG]: Create socket fd " << this->_server[i].get_fd() << " for port " << port << std::endl;
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

void Webserv::add_context(int fd, HttpHandler* context) {

    if (this->_context.find(fd) != this->_context.end())\
        throw std::runtime_error("[ERROR]: duplicated fd in context");

    this->_context[fd] = context;
}

// getter

// remover
void Webserv::remove_context(int fd) {

    std::map<int, HttpHandler*>::iterator it = this->_context.find(fd);

    if (it != this->_context.end())
        this->_context.erase(it);
}