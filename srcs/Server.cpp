#include "Server.hpp"

Server::Server(void) {

    // TODO: setup config file

    // get Listener
    get_listener();

    // start polling
    polling();

}

Server::~Server(void) {

}

// Get the listener fd
int Server::get_listener(void) {
    
    int listener;
    struct addrinfo hints, *ai, *p;

    // get socket
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // get the info from the host and port
    int rv;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        std::cout << "[ERROR]: getaddrinfo failed" << std::endl;
        return (-1);
    }

    // loop to find the available address and bind it
    for (p = ai; p != NULL; p = p->ai_next) {
        
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
            continue;

        // use for the last parameter
        int optval = 1;

        // set socket option to reuse the address
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
        
        // bind the address got from above
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        // got the available one, break the loop
        break;

    }

    // fre
    freeaddrinfo(ai);

    // check if there is no address available
    if (p == NULL)
        return (-1);
    
    // listening, with the backlog queue
    if (listen(listener, BACKLOG) == -1)
        return (-1);

    this->_listener = listener;

    std::cout << "[DEBUG]: system listening on fd " << listener << std::endl;

    return (0);
}

// polling
int Server::polling(void) {

    // create an epoll fd
    this->_epfd = epoll_create(1);
    if (this->_epfd == -1) {
        std::cout << "[ERROR]: Something wrong when creating an epoll fd" << std::endl;
        return (-1);
    }

    std::cout << "[DEBUG]: An epoll fd " << this->_epfd << " has been created" << std::endl;

    // put listener to be in the epollfd
    struct epoll_event ev, events[MAX_EVENTS];
    
    ev.events = EPOLLIN;
    ev.data.fd = this->_listener;
    
    if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, this->_listener, &ev) == -1) {
        std::cout << "[ERROR]: Something wrong when epoll_ctl adding the listener" << std::endl;
        return (-1);
    }

    // prepare for incoming connection
    struct sockaddr peer_addr;
    memset(&peer_addr, 0, sizeof(sockaddr));
    int peer_addrlen = sizeof(peer_addr);

    // start looping for listening
    while (1) {

        // epoll_wait to check any fd ready to do thing
        int nfds = epoll_wait(this->_epfd, events, MAX_EVENTS, -1);

        if (nfds == -1) {
            std::cout << "[ERROR]: Something wrong when epoll_wait" << std::endl;
            return (-1);
        }

        // loop through every event which available
        for (int i = 0; i < nfds; i++) {

            // fd is the listner, connect to it
            if (events[i].data.fd == this->_listener) {

                std::cout << "[INFO]: New connection found" << std::endl;

                int cfd = accept(this->_listener, (struct sockaddr*)&peer_addr, (socklen_t*)&peer_addrlen);
                if (cfd == -1) {
                    std::cout << "[ERROR]: Something wrong when tryong to accept the new connection" << std::endl;
                    continue;
                }

                // connection success, set fd to nonblock state
                setnonblock(cfd);

                // set it for reading state and add it to epoll fd
                // epoll_event nevent;
                // nevent.events = EPOLLIN | EPOLLET;
                // create the holder 

                // close it for no, just test
                close(cfd);

                // if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, cfd, &nevent) == -1) {
                //     std::cout << "[ERROR]: Something wrong when epoll_ctl adding the listener" << std::endl;
                //     close(cfd);
                // }

                std::cout << "[DEBUG]: Connection to " << cfd << " successfully" << std::endl;
            }
            // in case of EPOLLIN
            else if (events[i].events & EPOLLIN) {

            }
            // in case of EPOLLOUT
            else if (events[i].events & EPOLLOUT) {

            } else {
                std::cout << "[ERROR]: Some other case, can not be here at all" << std::endl;
            }

        }

    }

    return (0);
}

int Server::setnonblock(int fd) {
    return (fcntl(fd, F_SETFL, O_NONBLOCK));
}