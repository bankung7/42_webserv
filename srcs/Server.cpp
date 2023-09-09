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
    // this->_saddr.sin_addr = inet_addr("127.0.0.1"); // provide with ip later
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

    struct kevent change[4], event[4];
    int kq, nevent, socket_connection_fd;
    // prepare the kqueue
    kq = kqueue();

    EV_SET(change, this->_sfd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

    // register kevent
    if (kevent(kq, change, 1, NULL, 0, NULL) == -1)
        throw std::runtime_error("[ERROR] : kevent failed");

    // try connect with incoming
    for (;;) {
        nevent = kevent(kq, NULL, 0, event, 1, NULL);
        if (nevent == -1)
            throw std::runtime_error("[ERROR] : new event failed");

        for (int i = 0; nevent > i; i++) {
            int event_fd = event[i].ident;

            if (event[i].flags & EV_EOF) {
                std::cout << "Client has disconnected" << std::endl;
                close(event_fd);
            }
            else if (event_fd == this->_sfd) {
                socket_connection_fd = accept(event_fd, (sockaddr *)&this->_saddr, (socklen_t *)&this->_saddrLen);
                if (socket_connection_fd == -1) {
                    std::cout << "[ERROR] : accept failed to the connection" << std::endl;
                    continue ;
                }
                std::cout << "[DEBUG] : connection [" << socket_connection_fd << "] was accetped" << std::endl;

                EV_SET(change, socket_connection_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                if (kevent(kq, change, 1, NULL, 0, NULL) < 0)
                    std::cout << "[ERROR] : kevent error" << std::endl;
            }
            else if (event[i].filter & EVFILT_READ) {
                std::vector<char> bf(BUFFER_SIZE);

                int iread = recv(socket_connection_fd, bf.data(), BUFFER_SIZE, 0);
                if (iread < 0) {
                    std::cout << "[ERROR] : read nothing" << std::endl;
                    continue ;
                }

                // parsing and processing
                DataHolder holder(bf);

                // send status line and header
                send(socket_connection_fd, (void *)holder._header.c_str(), holder._headerLength, 0);

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
                    send(socket_connection_fd, (void *)bbf.data(), rd, 0);
                    bytesSend += rd;
                }

                // std::cout << "Total " << bytesSend << " was sent" << std::endl;
                std::cout << "======== Connection with [" << socket_connection_fd << "] closed ========" << std::endl;

                close(rd);
                // close(cfd);
                std::cout << std::endl;
            }
        }
    }
}
