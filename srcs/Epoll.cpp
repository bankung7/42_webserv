#include "Webserv.hpp"

// // Get the listener fd
// int Epoll::get_listener(int i) {

//     (void)i;
    
//     int listener;
//     struct addrinfo hints, *ai, *p;

//     // get socket
//     memset(&hints, 0, sizeof(hints));
//     hints.ai_family = AF_INET;
//     hints.ai_socktype = SOCK_STREAM;
//     hints.ai_flags = AI_PASSIVE;

//     // get the info from the host and port
//     // usage of getaddrinfo() https://linuxhint.com/c-getaddrinfo-function-usage/
//     int rv;
//     if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
//         std::cout << "[ERROR]: getaddrinfo failed" << std::endl;
//         return (-1);
//     }

//     // loop to find the available address and bind it
//     for (p = ai; p != NULL; p = p->ai_next) {
        
//         listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
//         if (listener < 0)
//             continue;

//         // use for the last parameter
//         int optval = 1;

//         // set socket option to reuse the address
//         setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
        
//         // bind the address got from above
//         if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
//             close(listener);
//             continue;
//         }

//         // got the available one, break the loop
//         break;

//     }

//     // fre
//     freeaddrinfo(ai);

//     // check if there is no address available
//     if (p == NULL)
//         return (-1);
    
//     // listening, with the backlog queue
//     if (listen(listener, BACKLOG) == -1)
//         return (-1);

//     this->_listener = listener;

//     std::cout << "[DEBUG]: system listening on fd " << listener << std::endl;

//     return (listener);
// }

// polling
int Webserv::polling(void) {

    // create an epoll fd
    this->_epfd = epoll_create(1);
    if (this->_epfd == -1) {
        std::cout << "[ERROR]: Something wrong when creating an epoll fd" << std::endl;
        return (-1);
    }

    std::cout << "[DEBUG]: An epoll fd " << this->_epfd << " has been created" << std::endl;

    // put listener to be in the epollfd
    struct epoll_event events[MAX_EVENTS];

    // add listener to epoll
    add_listener_to_epoll();

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

        std::cout << "[DEBUG]: found " << nfds << " ready" << std::endl;

        // loop through every event which available
        for (int i = 0; i < nfds; i++) {

            if (events[i].events & EPOLLIN) {

                // check if it match any listening

                // in the case of listner, create the new connection to it
                if (check_listener(events[i].data.fd) != -1) {

                    std::cout << "[INFO]: New connection found with " << events[i].data.fd << std::endl;

                    int conn = accept(events[i].data.fd, (struct sockaddr*)&peer_addr, (socklen_t*)&peer_addrlen);
                    if (conn == -1) {
                        std::cout << "[ERROR]: Something wrong when tryong to accept the new connection" << std::endl;
                        continue;
                    }

                    // connection success, set fd to nonblock state
                    setnonblock(conn);

                    // set it for reading state and add it to epoll fd
                    struct epoll_event nevent;
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
            }
            // in case of EPOLLOUT
            else if (events[i].events & EPOLLOUT) {

                HttpHandler* handler = (HttpHandler*)events[i].data.ptr;
                std::cout << "[DEBUG]: Epoll out state with fd " << handler->getfd() << std::endl;

                // TODO: sending state [HttpHandler.cpp]
                handler->handlingResponse();

                // remove from epollfd
                if (epoll_ctl(this->_epfd, EPOLL_CTL_DEL, handler->getfd(), NULL) == -1) {
                    std::cout << "[ERROR]: Something wrong when epoll_ctl removing the fd" << handler->getfd() << std::endl;
                }

                std::cout << "[DEBUG]: Close the connection with fd " << handler->getfd() << std::endl;

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

void Webserv::add_listener_to_epoll(void) {

    struct epoll_event ev;

    for (std::map<int, int>::iterator it = this->_socketList.begin(); it != this->_socketList.end(); ++it) {
        ev.events = EPOLLIN;
        ev.data.fd = it->first;

        std::cout << "[DEBUG]: add fd " << ev.data.fd << " to epoll fd" << std::endl;

        if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, it->first, &ev) == -1)
            throw std::runtime_error("epoll_ctl error for add");

    }

    std::cout << "[DEBUG]: succesfully add all listeners to epoll fd" << std::endl;

}
