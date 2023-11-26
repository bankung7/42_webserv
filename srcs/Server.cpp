#include "Server.hpp"

Server::Server(void) {

}

Server::~Server(void) {

}

int Server::start(void) {

    // read and set up server from config file
    setup_server();

    // get Listener
    get_listener(0);

    // loop
    // this->_port[0] = "8080";
    // this->_port[1] = "8081";

    // for (int i = 0; i < 2; i++) {
    //     this->_fd[i] = get_listener(i);
    //     // std::cout << "get " << i << " listener with fd " << this->_fd[i] << std::endl;
    // }

    // start polling
    polling();

    return (0);
}

// Get the listener fd
int Server::get_listener(int i) {

    (void)i;
    
    int listener;
    struct addrinfo hints, *ai, *p;

    // get socket
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // get the info from the host and port
    // usage of getaddrinfo() https://linuxhint.com/c-getaddrinfo-function-usage/
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

    return (listener);
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

    // loop put listener
    // for (int i = 0; i < 2; i++) {
    //     ev.events = EPOLLIN;
    //     ev.data.fd = this->_fd[i];
    //     if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, this->_fd[i], &ev) == -1) {
    //         std::cout << "[ERROR]: Something wrong when epoll_ctl adding the listener" << std::endl;
    //         return (-1);
    //     }
    // }

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
            // if (events[i].data.fd == this->_fd[0] || events[i].data.fd == this->_fd[1]) {

                std::cout << "[INFO]: New connection found with " << events[i].data.fd << std::endl;

                int conn = accept(events[i].data.fd, (struct sockaddr*)&peer_addr, (socklen_t*)&peer_addrlen);
                if (conn == -1) {
                    std::cout << "[ERROR]: Something wrong when tryong to accept the new connection" << std::endl;
                    continue;
                }

                // connection success, set fd to nonblock state
                setnonblock(conn);

                // set it for reading state and add it to epoll fd
                epoll_event nevent;
                nevent.events = EPOLLIN | EPOLLET;
                nevent.data.ptr = new HttpHandler(conn);

                if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, conn, &nevent) == -1) {
                    std::cout << "[ERROR]: Something wrong when epoll_ctl adding the listener" << std::endl;
                    close(conn);
                }

                std::cout << "[DEBUG]: Connection to " << conn << " successfully" << std::endl;
            }
            // in case of EPOLLIN
            else if (events[i].events & EPOLLIN) {

                HttpHandler* handler = (HttpHandler*)events[i].data.ptr;
                std::cout << "[DEBUG]: Epoll in state with " << handler->getfd() << std::endl;

                // TODO: reading state [HttpHandler.cpp]
                handler->handlingRequest();


                // reading complete, push to EPOLLOUT state
                epoll_event nevent;
                nevent.events = EPOLLOUT | EPOLLET;
                nevent.data.ptr = (void*)handler;

                if (epoll_ctl(this->_epfd, EPOLL_CTL_MOD, handler->getfd(), &nevent) == -1) {
                    std::cout << "[ERROR]: Something wrong when epoll_ctl modify the fd" << handler->getfd() << std::endl;
                }

            }
            // in case of EPOLLOUT
            else if (events[i].events & EPOLLOUT) {

                HttpHandler* handler = (HttpHandler*)events[i].data.ptr;
                std::cout << "[DEBUG]: Epoll out state with " << handler->getfd() << std::endl;

                // TODO: sending state [HttpHandler.cpp]
                handler->handlingResponse();

                // remove from epollfd
                if (epoll_ctl(this->_epfd, EPOLL_CTL_DEL, handler->getfd(), NULL) == -1) {
                    std::cout << "[ERROR]: Something wrong when epoll_ctl removing the fd" << handler->getfd() << std::endl;
                }

                std::cout << "[DEBUG]: Close the connection with " << handler->getfd() << std::endl;

                // clear the context
                close(handler->getfd());
                delete handler;


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

void Server::set_config_name(std::string file) {
    this->_config_file = std::string(file);
}

void Server::setup_server(void) {




}