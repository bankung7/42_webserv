#include "Server.hpp"

Server::Server(void) {
}

Server::~Server(void) {
}

void Server::setConfigFile(std::string file) {
    (void)file;
    std::cout << "[DEBUG] : set config file path [" << file << "] => planning" << std::endl;
}

void Server::start(void) {

    // load and set config

    // create server socket
    this->_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_sfd == -1)
        throw std::runtime_error("[ERROR] : (socket failed)");
    std::cout << "[DEBUG] : socket has been created successfully" << std::endl;

    // clear the add first
    memset(&this->_saddr, 0, sizeof(this->_saddr));

    // bind ip and port
    this->_saddr.sin_family = AF_INET;
    this->_saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    this->_saddr.sin_port = htons(PORT);
    
    // set length
    this->_saddrLen = sizeof(this->_saddr);

    // set to res use the address, prevent waiting for old address killed in last process
    if (setsockopt(this->_sfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&this->_saddr, this->_saddrLen) < 0)
        throw std::runtime_error("[ERROR] : set socket address reuse failed");

    // bind socket with port
    if (bind(this->_sfd, (sockaddr *)&this->_saddr, this->_saddrLen) != 0)
        throw std::runtime_error("[ERROR] : (bind failed)");
    std::cout << "[DEBUG] : bind was successfully" << std::endl;

    // listen
    if (listen(this->_sfd, SOMAXCONN) != 0)
        throw std::runtime_error("[ERROR] : listen failed");
    std::cout << "[DEBUG] : listen is ready" << std::endl;

    for (;;) {

        int client_fd = accept(this->_sfd, (sockaddr *)&this->_saddr, (socklen_t *)&this->_saddrLen);
        if (client_fd < 0) {
            std::cout << "[ERROR] : accept failed" << std::endl;
            continue ;
        }

        std::cout << "[DEBUG] : connection successfully " << client_fd << std::endl;

        std::vector<char> bf(BUFFER_SIZE);

        int iread = recv(client_fd, bf.data(), BUFFER_SIZE, 0);
        if (iread < 0) {
            std::cout << "[ERROR] : read nothing" << std::endl;
            close(client_fd);
            continue ;
        }

        // parsing and processing
        DataHolder holder(bf);

        // send status line and header
        send(client_fd, (void *)holder._header.c_str(), holder._headerLength, 0);

        int bytesSend = 0;
        int ifile = open(holder._filename.c_str(), O_RDONLY);
        int rd;

        // loop send body
        while (bytesSend < holder._bodyLength) {
            std::vector<char> bbf(BUFFER_SIZE);
            rd = read(ifile, bbf.data(), BUFFER_SIZE);
            if (rd == -1) {
                std::cout << "[ERROR] : Something wrong when tring to send data" << std::endl;
                break ;
            }
            send(client_fd, (void *)bbf.data(), rd, 0);
            bytesSend += rd;
        }

        // std::cout << "Total " << bytesSend << " was sent" << std::endl;
        std::cout << "======== Connection with [" << client_fd << "] closed ========" << std::endl;

        close(rd);
        close(client_fd);
    }
}
