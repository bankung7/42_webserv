#include "Webserv.hpp"

Webserv::Webserv(void): _backlog(20) {

    std::cout << "[DEBUG]: Webserv initiated" << std::endl;

    // Manually create the server block
    Server sv1, sv2, sv3;
    // sv1.add_server_name("webserv1");
    sv1.set_port(8080);
    sv1.set_location("/", "allowedMethod:GET;root:/sites/www1;"); // default index
    sv1.set_location("/html","allowedMethod:GET;root:/sites/www1;index:defined.html;error_page: 500 501 501 505 /error_50x.html;error_page: 404 /error_404.html;error_page: 405 /error_405.html;"); // defined index 
    sv1.set_location("/files","allowedMethod:GET;root:/sites/www1;autoIndex:on;"); // autoindex, if defined index, it would be index
    sv1.set_location("/google", "allowedMethod:GET;return:https://www.google.co.th;"); // for redirection
    sv1.set_location("/box/", "allowedMethod:GET,DELETE;root:/sites/www1;"); // for DELETE
    sv1.set_location("/uploads", "allowedMethod:POST;root:/sites/www1/temp;allowedFileUpload:yes;"); // for uploads, so use the root to tell where to save
    add_server(sv1);

    sv2.add_server_name("webserv11");
    sv2.add_server_name("webserv111");
    sv2.set_port(8080);
    sv2.set_location("/", "allowedMethod:GET;root:/sites/www2;index:index.html;");
    add_server(sv2);

    sv3.add_server_name("webserv2");
    sv3.set_port(8081);
    sv3.set_location("/", "allowedMethod:GET;root:/sites/www3;index:index.html;");
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