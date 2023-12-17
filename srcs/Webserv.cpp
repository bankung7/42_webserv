#include "Webserv.hpp"

Webserv::Webserv(void) {
    this->_serverSize = 0;

}

Webserv::~Webserv(void) {

}

// Configuration Part
void Webserv::set_config_name(std::string file) {
    this->_configFile = std::string(file);
}

void Webserv::start(void) {

    // read and set up server from config file
    setup();

    // TODO: create socket
    create_socket(this->_server);

    std::cout << "[DEBUG]: Created all listener [" << this->_serverSize << "] completed" << std::endl;

    // start polling
    polling();

}

// create socket
void Webserv::create_socket(std::vector<Server> &server) {

    int listening;

    // loop create with total size
    for (int i = 0; i < this->_serverSize; i++) {

        // std::string cip = server[i].get_ip(); // for specified ip
        int cport = server[i].get_port();

        std::cout << "[DEBUG]: Start creating socket for port " << cport << std::endl;

        // create socket
        listening = socket(AF_INET, SOCK_STREAM, 0);
        if (listening == -1)
            throw std::runtime_error("[ERROR]: failed to create new socket");

        // setsockopt for reusing
        int optval = 1;
        setsockopt(listening, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(int));
        // reuse port for same port but different in server name ?

        // set address
        struct sockaddr_in  addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(cport);
        addr.sin_addr.s_addr = INADDR_ANY; // for 0.0.0.0
        // addr.sin_addr.s_addr = inet_addr(cip.c_str()); // for specific ip

        // bind the address with port
        // if (bind(this->_socket[current], (struct sockaddr*)&this->_addr[current], (int)this->_addrLen[current]) == -1)
        if (bind(listening, (struct sockaddr*)&addr, (int)(sizeof(addr))) == -1)
            throw std::runtime_error("[ERROR]: failed to bind the port");

        // set socket and its addr
        server[i].set_socket(listening); // keep to its own data
        server[i].set_addr(addr);
        server[i].set_addr_len(sizeof(addr));

        // add to webser for checking in epoll state
        add_socket(listening);
        
        // set socket as nonblock state
        setnonblock(listening);

        // listening
        if (listen(listening, BACKLOG) == -1)
            throw std::runtime_error("failed to listen the socket");
        
        // complete
        std::cout << "[DEBUG]: fd " << listening << " on port " << cport << " starting" << std::endl;

    }
}

int Webserv::setnonblock(int fd) {
    return (fcntl(fd, F_SETFL, O_NONBLOCK));
}

// check listen and server_name
int Webserv::check_listener(int fd) {
    for (int i = 0; i < this->_serverSize; i++) {
        if (fd == this->_socketList[i])
            return (i);
    }
    return (-1);
}

// setter
void Webserv::add_socket(int fd) {
    this->_socketList.push_back(fd);
}

// getter
Server& Webserv::get_server(int i) {
    return (this->_server[i]);
}

// Test function
void Webserv::setup(void) {

    std::cout << "[DEBUG]: setting up" << std::endl;

    this->_serverSize = 3;

    // default server
    Server sv1;
    sv1.set_port(8080);
    sv1.add_server_name("webserv1");
    sv1.set_root("/sites/www1/");
    this->_server.push_back(sv1);

    Server sv2;
    sv2.set_port(8080);
    sv2.add_server_name("webserv2");
    sv2.set_root("/sites/www2/");
    this->_server.push_back(sv2);

    Server sv3;
    sv3.set_port(8081);
    sv3.add_server_name("webserv3");
    sv3.set_root("/sites/www3/");
    this->_server.push_back(sv3);

}
