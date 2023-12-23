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

        // std::cout << nfds << std::endl;

        for (int i = 0; i < nfds; i++) {
            
            struct epoll_event event = events[i];

            // EPOLLIN event
            if (events[i].events & EPOLLIN) {

                // std::cout << "[INFO]: event fd " << event.data.fd << " ready" << std::endl;

                // if fd, new connection
                if (this->_fd.find(event.data.fd) != this->_fd.end()) {
                    int conn = accept(event.data.fd, (struct sockaddr*)&caddr, (socklen_t*)&caddrLen);
                    if (conn == -1) {
                        perror("failed");
                        throw std::runtime_error("[ERROR]: accept failed");
                    }
                    
                    // set nonblocking
                    if (fcntl(conn, F_SETFL, O_NONBLOCK) == -1)
                        throw std::runtime_error("[ERROR]: set non-blocking failed");

                    std::cout << "[DEBUG]: New connection to " << conn << std::endl;

                    // set to EPOLLIN state for reading

                    // create context
                    HttpHandler* context = new HttpHandler(conn);
                    // this->add_context(conn, context);
                    event.events = EPOLLIN;
                    // ev.data.fd = conn;
                    event.data.ptr = (void*)context;

                    if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, conn, &event) == -1)
                        throw std::runtime_error("[ERROR]: epoll add to epfd failed");

                    std::cout << "[DEBUG]: Set " << conn << " to EPOLLIN state" << std::endl;

                } else {
                    // EPOLLIN job, reading state
                    std::cout << "[DEBUG]: reading state" << std::endl;

                    // get context
                    HttpHandler* context = (HttpHandler*)event.data.ptr;

                    // set to EPOLLOUT state for writing
                    event.events = EPOLLOUT;
                    event.data.ptr = (void*)context;
                    // event.data.fd = events[i].data.fd;
                    if (epoll_ctl(this->_epfd, EPOLL_CTL_MOD, context->get_fd(), &event) == -1)
                        throw std::runtime_error("[ERROR]: epoll mod to epfd failed");

                }

            } else if (event.events & EPOLLOUT) { // EPOLLOUT event
                std::cout << "[DEBUG]: writing state" << std::endl;

                // get context
                HttpHandler* context = (HttpHandler*)event.data.ptr;

                std::stringstream sres;
                sres << "HTTP/1.1 200 OK\r\n"
                        << "Content-type: text/html\r\n"
                        << "Content-Length: 5\r\n\r\n"
                        << "Hello";

                std::string ssres = sres.str();
                std::vector<char> res;
                res.insert(res.begin(), ssres.begin(), ssres.end());

                // int client_fd = event.data.fd;
                int client_fd = context->get_fd(); // get client fd
                if (send(client_fd, res.data(), res.size(), 0) == -1)
                    throw std::runtime_error("[ERROR]: send error");

                if (epoll_ctl(this->_epfd, EPOLL_CTL_DEL, client_fd, NULL) == -1)
                    throw std::runtime_error("[ERROR]: epoll del to epfd failed");

                std::cout << "[DEBUG]: close the connection" << std::endl;
                this->remove_context(client_fd);
                close(client_fd);
            } else {
                // not any case
            }
        }
    }

    return (0);
};