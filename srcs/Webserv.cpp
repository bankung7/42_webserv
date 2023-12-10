#include "Webserv.hpp"

Webserv::Webserv(void) {
    this->_serverSize = 0;
    this->_socketSize = 0;

}

Webserv::~Webserv(void) {

}

void Webserv::start(void) {

    // read and set up server from config file
    setup();

    // loop creating the socket
    // TODO: after parsing config file, set the serverSize
    for (int i = 0; i < this->_serverSize; i++) {

        // for each port in each server
        int size = this->_server[i].get_size();
        std::cout << "[DEBUG]: size of " << this->_server[i].get_server_name() << " = " << size << std::endl;

        // TODO: bind with hostname
        for (int j = 0; j < size; j++)
            create_socket(this->_server[i].get_host(), this->_server[i].get_port(j), size);
        
    }

    // start polling
    polling();

}

// Test function
void Webserv::setup(void) {

    std::cout << "[DEBUG]: setting up" << std::endl;

    this->_serverSize = 2;

    Server sv1;
    sv1.add_host("127.0.0.1");
    sv1.add_port(8081);
    sv1.set_size(1);
    sv1.set_server_name("test1");
    this->_server.push_back(sv1);


    Server sv2;
    sv2.add_host("127.0.0.2");
    sv2.add_port(8082);
    sv2.set_size(1);
    sv2.set_server_name("test2");
    this->_server.push_back(sv2);

}

void Webserv::set_config_name(std::string file) {
    this->_configFile = std::string(file);
}

int Webserv::check_listener(int fd) {
    for (int i = 0; i < this->_serverSize; i++) {
        if (this->_socket[i] == fd)
            return (fd);
    }
    return (-1);
}

// create socket
void Webserv::create_socket(std::string host, int port, int size) {

    int listening;

    // loop create with total size
    for (int i = 0; i < size; i++) {

        std::cout << "[DEBUG]: Start creating socket for " << host << ":" << port << std::endl;

        // create socket
        listening = socket(AF_INET,SOCK_STREAM, 0);
        if (listening == -1)
            throw std::runtime_error("[ERROR]: failed to create new socket");

        // add listening to the socket group
        this->_socket.push_back(listening);
        this->_socketSize++;

        std::cout << "[DEBUG]: socket " << listening << " was created" << std::endl;

        // setsockopt for reusing
        int optval = 1;
        setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

        // set address
        struct sockaddr_in  addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        // addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_addr.s_addr = inet_addr(host.c_str());

        // push the address to the gorup
        this->_addr.push_back(addr);
        this->_addrLen.push_back(sizeof(addr));

        // bind the address with port
        int current = this->_socketSize - 1;
        if (bind(this->_socket[current], (struct sockaddr*)&this->_addr[current], (int)this->_addrLen[current]) == -1)
            throw std::runtime_error("[ERROR]: failed to bind the port");
        
        // set socket as nonblock state
        setnonblock(this->_socket[current]);

        // listening
        if (listen(this->_socket[current], BACKLOG) == -1)
            throw std::runtime_error("failed to listen the socket");
        
        // complete
        std::cout << "[DEBUG]: The fd " << this->_socket[current] << " starting listening on " << host << ":" << port << std::endl;

    }
}

int Webserv::setnonblock(int fd) {
    return (fcntl(fd, F_SETFL, O_NONBLOCK));
}
