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
        int nfds = epoll_wait(this->_epfd, events, MAX_EVENTS, 1000);

        // std::cout << "epoll loop" << std::endl;

        // if error occured
        if (nfds == -1)
            throw std::runtime_error("[ERROR]: epoll_wait failed");

        for (int i = 0; i < nfds; i++) {
            
            struct epoll_event event = events[i];

            // EPOLLIN event
            if (events[i].events & EPOLLIN) {

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

                    std::cout << "\033[0;32m[DEBUG]: New connection initiated wtih " << conn << " from " << event.data.fd << "\033[0m" << std::endl;

                    // create context and set to EPOLLIN
                    HttpHandler* context = new HttpHandler(conn);
                    event.events = EPOLLIN;
                    event.data.ptr = (void*)context;

                    add_context(context->get_fd(), context); /// old
                    // this->_client.push_back(context); // new

                    context->set_server(this->_server);

                    if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, conn, &event) == -1)
                        throw std::runtime_error("[ERROR]: epoll add to epfd failed");

                    std::cout << "[DEBUG]: Set " << conn << " to EPOLLIN state" << std::endl;

                } 
                // EPOLLIN, reading state
                else {

                    HttpHandler* context = (HttpHandler*)event.data.ptr;

                    std::cout << "[DEBUG]: reading state for " << context->get_fd() << std::endl;

                    // start reading and not finish yet
                    if (context->get_status() <= READING) {

                        context->handle_request();

                        // if completed for a 1 call
                        if (context->get_status() == WRITING) {
                            event.events = EPOLLOUT;
                            event.data.ptr = (void*)context;
                            if (epoll_ctl(this->_epfd, EPOLL_CTL_MOD, context->get_fd(), &event) == -1)
                                throw std::runtime_error("[ERROR]: epoll mod [OUT] to epfd failed");
                        }
                        // no yet, back to epollin and re-read
                        continue ;
                    }

                    // in case the connection was closed from client
                    if (context->get_status() == CLOSED) {
                        std::cout << "[DEBUG]: Connection was closed from client" << std::endl;
                        close_connection(context); //======================== CLOSE CONNECTION
                        continue ;
                    }

                }

            } else if (event.events & EPOLLOUT) { // EPOLLOUT event

                // get context
                HttpHandler* context = (HttpHandler*)event.data.ptr;

                std::cout << "[DEBUG]: writing state for " << context->get_fd() << std::endl;

                context->handle_response();

                std::cout << "[DEBUG]: Connection type: " << context->get_connection_type() << std::endl;

                // if the connection was closed 
                if (context->get_connection_type().find("closed") == 0) {
                    std::cout << "Closed connection after finishing" << std::endl;
                    close_connection(context); //============================= CLOSE CONNECTION
                    continue;
                }

                // this mean completed phase reached
                // check the connection it is keep-alive or not
                // if yes, reset context and put to EPOLLIN
                if (context->get_status() == COMPLETE_PHASE) {

                    std::time_t ctime;
                    std::time(&ctime);

                    if (context->get_time_out() > ctime) {
                        // reset this client data to put in the wait queue
                        HttpHandler *newContext = new HttpHandler(context->get_fd());
                        this->_context[context->get_fd()] = newContext; // old
                        newContext->set_server(this->_server);

                        delete context;

                        event.events = EPOLLIN;
                        event.data.ptr = (void*)newContext;

                        if (epoll_ctl(this->_epfd, EPOLL_CTL_MOD, newContext->get_fd(), &event) == -1) {
                            perror("epoll del timeout");
                            throw std::runtime_error("[ERROR]: epoll mod to epfd failed when reset context");
                        }
                        
                        // std::cout << "=== client " << newContext->get_fd() << " reset to wait queue ===" << std::endl;

                        continue ;
                    }
                }

            } else if ((event.events & EPOLLHUP) || (event.events & EPOLLERR)) { // EPOLLOUT event
            
                // for EPOLLERR and EPOLLHUP, throw them out, and shutdown
                throw std::runtime_error("[ERROR]: EPOLLERR or EPOLLHUP");

            } else {
                // not any case
                std::cout << "or may be this" << std::endl;
            }

        }

        // check time out
        check_time_out();
    }

    return (0);
};

void Webserv::close_connection(HttpHandler* context) {

    // TODO:
    int client_fd = context->get_fd(); // get client fd
    std::cout << "[DEBUG]: Preapared to close the connection with " << client_fd << std::endl;

    // if it can be find in the context // weird
    if (this->_context.find(client_fd) != this->_context.end()) {

        if (epoll_ctl(this->_epfd, EPOLL_CTL_DEL, client_fd, NULL) == -1) {
            perror("epoll delete");
            throw std::runtime_error("[ERROR]: epoll del to epfd failed from closed connection");
        }
        this->_context.erase(client_fd); // remove from the map
        delete context; // delete it
        close(client_fd); // close fd, so server can give this fd to other for now

        std::cout << "\033[;33m[DEBUG]: close the connection with " << client_fd << "\033[0m" << std::endl;
    }

}

void Webserv::check_time_out(void) {

    // std::cout << "\033[0;31mstart check time out\033[0;0m" << std::endl;
    
    std::time_t ctime;
    time(&ctime);

    std::vector<int> fd_list;

    std::map<int, HttpHandler*>::iterator it = this->_context.begin();

    // check and put the timeout into the list, then clear it below.
    for (; it != this->_context.end(); it++) {

        HttpHandler* cont = it->second;

        std::cout << "[DEBUG]: client [" << cont->get_fd() << "] : status [" << cont->get_status() << "]";
        std::cout << " time out in " << cont->get_time_out() - ctime << std::endl;

        if (cont->get_status() < 2) {
            if (cont->get_time_out() < ctime) {
                std::cout << "[DEBUG]: client " << cont->get_fd() << " has time out" << std::endl;
                fd_list.push_back(it->first);
            }
        }
    }
    
    // loop erase that expired, and remove it
    for (int i = 0; i < (int)fd_list.size(); i++) {
        close_connection(this->_context[fd_list[i]]);
    }

}
