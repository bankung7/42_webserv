#include "Server.hpp"

Server::Server(void) {

    // start
    try {
        start();
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

}

Server::~Server(void) {

}

void Server::start(void) {

    // create server socket
    this->_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_sfd == -1)
        throw std::runtime_error("[ERROR] : (socket failed)");
    std::cout << "[DEBUG] : socket has been created successfully" << std::endl;

    // bind ip and port
    this->_saddr.sin_family = AF_INET;
    this->_saddr.sin_port = htons(PORT);
    this->_saddr.sin_addr.s_addr = htonl(INADDR_ANY);
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

    // try connect with incoming
    while (1) {

        // accept the request as fd
        int cfd = accept(this->_sfd, (sockaddr *)&this->_saddr, (socklen_t *)&this->_saddrLen);
        if (cfd < 0) {
            std::cout << "[ERROR] : accept failed to the connection" << std::endl;
            continue ;
        }
        std::cout << "[DEBUG] : connection [" << cfd << "] was accetped" << std::endl;

        // read incoming request
        std::vector<char> bf(BUFFER_SIZE);

        int iread = recv(cfd, bf.data(), BUFFER_SIZE, 0);
        if (iread < 0) {
            std::cout << "[ERROR] : read nothing" << std::endl;
            continue ;
        }

        // parsing and processing
        DataHolder holder(bf);

        // send status line and header
        send(cfd, (void *)holder._header.c_str(), holder._headerLength, 0);

        int bytesSend = 0;
        int ifile = open(holder._filename.c_str(), O_RDONLY);
        int rd;
    
        // loop send body
        while (bytesSend < holder._bodyLength) {
            std::vector<char> bbf(BUFFER_SIZE);
            rd = read(ifile, bbf.data(), BUFFER_SIZE);
            if (rd == -1) {
                std::cout << "may be end of file" << std::endl;
                break ;
            }
            send(cfd, (void *)bbf.data(), rd, 0);
            bytesSend += rd;
        }

        std::cout << "Total " << bytesSend << " was sent" << std::endl;

        close(rd);
        close(cfd);
        std::cout << std::endl;
    }
}
