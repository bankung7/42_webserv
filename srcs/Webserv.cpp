#include "Webserv.hpp"

static void signal_handler(int sig) {
    (void)sig;
    signal(SIGINT, SIG_IGN);
    throw (0);
}

Webserv::Webserv(void): _backlog(20) {

    std::cout << "[DEBUG]: Webserv initiated" << std::endl;

    signal(SIGINT, signal_handler);
    // signal(SIGPIPE, SIG_IGN); // broken pipe when siege
    
    // for main loop
    try {
		Conf cf;
		cf.parseconf(this->_server);
        setup();
        polling();
    } catch (int e) {
        switch (e) {
            case (0):
                std::cout << B_YELLOW << "Shutdown signal detected" << C_RESET << std::endl;
                break ;
            case (-1):
                std::cout << B_RED << "Something wrong happed" << C_RESET << std::endl;
                break ;
            default:
                break ;
        }
        // exit signal
        std::cout << B_YELLOW << "\n[WARNING]: Prepare closing the server" << C_RESET << std::endl;
    }

    // cleaning
    try {
        clean_context();
        clean_socket();
        close(this->_epfd);
    } catch (...) {

    }

    std::cout << B_GREEN << "[INFO]: Exit completed" << C_RESET << std::endl;
}

Webserv::Webserv(std::string filename){
    Conf cf(filename);
}

Webserv::~Webserv(void) {

}

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
            std::cout << "[DEBUG]: Create socket [" << this->_server[i].get_fd() << "] for port [" << port << "]" << std::endl;
            continue ;
        }
       
       // Port Duplication case
       this->_log.clear();
       this->_log << "[ERROR]: Port [" << port << "] is duplicatied";
       throw std::runtime_error(this->_log.str());

    }
};

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

// error handling part
void Webserv::clean_socket(void) {

    std::set<int>::iterator it = this->_fd.begin();

    for (; it != this->_fd.end(); it++) {
        std::cout << B_YELLOW << "[DEBUG]: server socket " << *it << " is being closed now" << C_RESET << std::endl;
        epoll_del(*it); // remove it from epoll events
        close(*it);
    }

    this->_fd.clear();
}

void Webserv::clean_context(void) {

    std::map<int, HttpHandler*>::iterator it = this->_context.begin();

    for (; it != this->_context.end(); it++) {
        std::cout << B_YELLOW << "[DEBUG]: Context for client " << it->first << " is being closed now" << C_RESET << std::endl;
        epoll_del(it->first); // remove it from epoll events
        close(it->first); // close client socket
        delete it->second; // delete the client context
        // std::cout << "delete context for fd " << it->first << std::endl;
    }
    
    this->_context.clear();
}
