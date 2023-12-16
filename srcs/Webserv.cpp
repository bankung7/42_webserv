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

    std::cout << "[DEBUG]: Created all listener completed" << std::endl;

    // start polling
    polling();

}

// create socket
void Webserv::create_socket(std::vector<Server> &server) {

    int listening;

    // loop create with total size
    for (int i = 0; i < this->_serverSize; i++) {

        std::string cip = server[i].get_ip();
        int cport = server[i].get_port();

        std::cout << "[DEBUG]: Start creating socket for port " << cport << std::endl;

        // create socket
        listening = socket(AF_INET, SOCK_STREAM, 0);
        if (listening == -1)
            throw std::runtime_error("[ERROR]: failed to create new socket");

        // set and add listening
        server[i].set_socket(listening); // keep to its own data
        add_socket(listening); // add to Webserv for searching

        // setsockopt for reusing
        int optval = 1;
        setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

        // set address
        struct sockaddr_in  addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(cport);
        addr.sin_addr.s_addr = INADDR_ANY; // for 0.0.0.0
        // addr.sin_addr.s_addr = inet_addr(host.c_str()); // for specific ip

        // set addr and it length
        // this->_addr.push_back(addr);
        // this->_addrLen.push_back(sizeof(addr));
        server[i].set_addr(addr);
        server[i].set_addr_len(sizeof(addr));

        // bind the address with port
        // if (bind(this->_socket[current], (struct sockaddr*)&this->_addr[current], (int)this->_addrLen[current]) == -1)
        if (bind(listening, (struct sockaddr*)&addr, (int)(sizeof(addr))) == -1)
            throw std::runtime_error("[ERROR]: failed to bind the port");
        
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


int Webserv::check_listener(int fd) {
    
    std::cout << "[DEBUG]: cheking listener" << std::endl;
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

// Test function
void Webserv::setup(void) {

    std::cout << "[DEBUG]: setting up" << std::endl;

    this->_serverSize = 2;

    Server sv1;
    // sv1.add_host("127.0.0.1");
    sv1.set_port(8081);
    sv1.add_server_name("test1");
    this->_server.push_back(sv1);

    Server sv2;
    // sv2.add_host("127.0.0.9");
    sv2.set_port(8082);
    sv2.add_server_name("test2");
    this->_server.push_back(sv2);

}
