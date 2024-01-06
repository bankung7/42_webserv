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
        int nfds = epoll_wait(this->_epfd, events, MAX_EVENTS, 500);

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
                    this->_client.push_back(context); // new

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

                        // if completed
                        if (context->get_status() == WRITING) {
                            event.events = EPOLLOUT;
                            event.data.ptr = (void*)context;
                            if (epoll_ctl(this->_epfd, EPOLL_CTL_MOD, context->get_fd(), &event) == -1)
                                throw std::runtime_error("[ERROR]: epoll mod [OUT] to epfd failed");
                        }

                        continue ;
                    }

                    // in case of conection was hangup
                    if (context->get_status() == WRITING) {
                        std::cout << "writing state" << std::endl;

                        // reading complete send to EPOLLOUT
                        event.events = EPOLLOUT;
                        event.data.ptr = (void*)context;
                        if (epoll_ctl(this->_epfd, EPOLL_CTL_MOD, context->get_fd(), &event) == -1)
                            throw std::runtime_error("[ERROR]: epoll mod [OUT] to epfd failed");

                        std::cout << "[DEBUG]: Reading finish, sent to EPOLLOUT" << std::endl;
                    } 
                    // in case the connection was closed from client
                    else if (context->get_status() == CLOSED) {
                        std::cout << "[DEBUG]: Connection was closed from client" << std::endl;
                        close_connection(context);
                    }
                    else 
                        std::cout << "[DEBUG]: reading in progress for next buffer" << std::endl;

                    // continue reading;

                }
            } else if (event.events & EPOLLOUT) { // EPOLLOUT event

                // get context
                HttpHandler* context = (HttpHandler*)event.data.ptr;

                std::cout << "[DEBUG]: writing state for " << context->get_fd() << std::endl;

                context->handle_response();

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

                        remove_client(context->get_fd()); // remove old
                        this->_client.push_back(newContext); // new

                        delete context;

                        event.events = EPOLLIN;
                        event.data.ptr = (void*)newContext;

                        if (epoll_ctl(this->_epfd, EPOLL_CTL_MOD, newContext->get_fd(), &event) == -1) {
                            perror("epoll del timeout");
                            throw std::runtime_error("[ERROR]: epoll mod to epfd failed");
                        }
                        
                        // std::cout << "=== client " << newContext->get_fd() << " reset to wait queue ===" << std::endl;

                        continue ;
                    }
                }

                // if no, close the connection now
                // std::cout << "[DEBUG]: Connection will be closed now as " << context->get_connection_type() << std::endl;
                close_connection(context);

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

    if (epoll_ctl(this->_epfd, EPOLL_CTL_DEL, client_fd, NULL) == -1) {
        perror("epoll delete");
        throw std::runtime_error("[ERROR]: epoll del to epfd failed");
    }

    std::cout << "\033[;33m[DEBUG]: close the connection with " << client_fd << "\033[0m" << std::endl;
    this->remove_context(client_fd);
    close(client_fd);

}

void Webserv::check_time_out(void) {

    // std::cout << "\033[0;31mstart check time out\033[0;0m" << std::endl;
    
    std::time_t ctime;
    time(&ctime);

    for (int i = 0; i < (int)this->_client.size(); i++) {

        // std::cout << "[DEBUG]: client [" << this->_client[i]->get_fd() << "] : status [" << this->_client[i]->get_status() << "]";
        // std::cout << " time out in " << this->_client[i]->get_time_out() - ctime << std::endl;

        if (this->_client[i]->get_status() < 2) {
            if (this->_client[i]->get_time_out() < ctime) {
                std::cout << "[DEBUG]: client " << this->_client[i]->get_fd() << " has time out" << std::endl;
                close_connection(this->_client[i]);
                _client.erase(_client.begin() + i);
                i--;
            }
        }
    }

}

void Webserv::remove_client(int fd) {

    for (int i = 0; i < (int)this->_client.size(); i++) {
        if (this->_client[i]->get_fd() == fd) {
            this->_client.erase(this->_client.begin() + i);
            return ;
        }
    }
}