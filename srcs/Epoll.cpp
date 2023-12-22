#include "Webserv.hpp"

int Webserv::polling(void) {
    
    // create epoll fd
    this->_epfd = epoll_create(1);
    if (this->_epfd == -1)
        throw std::runtime_error("[ERROR]: epoll_create failed");
    
    // add all listener to epfd
    struct epoll_event ev;
    for (std::set<int>::iterator it = this->_fd.begin(); it != this->_fd.end(); it++) {
        ev.events = EPOLLIN;
        ev.data.fd = *it;
        if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, *it, &ev) == -1)
            throw std::runtime_error("[ERROR]: epoll add to epfd failed");
    }

    std::cout << "[DEBUG]: The server starting polling" << std::endl;

    // loop
    struct epoll_event events[MAX_EVENTS];
    struct sockaddr caddr;
    socklen_t caddrLen = sizeof(caddr);

    while (1) {
        int nfds = epoll_wait(this->_epfd, events, MAX_EVENTS, -1);

        // if error occured
        if (nfds == -1)
            throw std::runtime_error("[ERROR]: epoll_wait failed");

        for (int i = 0; i < nfds; i++) {
            // EPOLLIN event
            if (events[i].events & EPOLLIN) {

                // if fd, new connection
                
                std::cout << "[DEBUG]: fd ready " << events[i].data.fd << std::endl;
                if (this->_fd.find(events[i].data.fd) != this->_fd.end()) {
                    int conn = accept(events[i].data.fd, (struct sockaddr*)&caddr, (socklen_t*)&caddrLen);
                    if (conn == -1)
                        throw std::runtime_error("[ERROR]: accept failed");
                    
                    // set nonblocking
                    if (fcntl(conn, F_SETFL, O_NONBLOCK) == -1)
                        throw std::runtime_error("[ERROR]: set non-blocking failed");

                    std::cout << "[DEBUG]: New connection to " << conn << std::endl;

                    // set to EPOLLIN state for reading
                    ev.events = EPOLLIN;
                    ev.data.fd = conn;
                    if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, conn, &ev) == -1)
                        throw std::runtime_error("[ERROR]: epoll add to epfd failed");

                } else {
                    // EPOLLIN job, reading state
                    std::cout << "[DEBUG]: reading state" << std::endl;

                    // set to EPOLLOUT state for writing
                    ev.events = EPOLLOUT;
                    ev.data.fd = events[i].data.fd;
                    if (epoll_ctl(this->_epfd, EPOLL_CTL_MOD, events[i].data.fd, &ev) == -1)
                        throw std::runtime_error("[ERROR]: epoll mod to epfd failed");

                }

            } else if (events[i].events & EPOLLOUT) { // EPOLLOUT event
                std::cout << "[DEBUG]: writing state" << std::endl;

                if (epoll_ctl(this->_epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1)
                    throw std::runtime_error("[ERROR]: epoll del to epfd failed");

                std::cout << "[DEBUG]: close the connection" << std::endl;
                close(events[i].data.fd);
            } else {
                // not any case
            }
        }
    }

    return (0);
};