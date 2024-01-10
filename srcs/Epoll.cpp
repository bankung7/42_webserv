#include "Webserv.hpp"

int Webserv::polling(void) {

    // create epoll instance
    this->_epfd = epoll_create(1);
    if (this->_epfd == -1) {
        std::cout << B_RED << "[ERROR]: epoll_create() failed for some reason" << C_RESET << std::endl;
        throw (-1);
    }

    // add all server listener to epoll list
    struct epoll_event ev;
    for (std::set<int>::iterator it = this->_fd.begin(); it != this->_fd.end(); it++) {
        ev.events = EPOLLIN;
        ev.data.fd = *it;
        if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, *it, &ev) == -1) {
            std::cout << B_RED << "[ERROR]: failed to add listener to epoll list" << C_RESET << std::endl;
            throw (-1);
        }
        std::cout << B_PURPLE << "[INFO]: " << *it << " is on the epoll list" << C_RESET << std::endl;
    }

    std::cout << B_PURPLE << "[INFO]: Starting Epoll loop now" << C_RESET << std::endl;
    
    // create variable
    struct epoll_event events[MAX_EVENTS];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int nfds;

    while (1) {

        nfds = epoll_wait(this->_epfd, events, MAX_EVENTS, -1);

        // if wait error
        if (nfds == -1) {
            if (errno == EINTR) {
                std::cout << B_YELLOW << "[WARNING]: epoll_wait was interupted for some reason, will procedd in next loop" << C_RESET << std::endl;
                continue ;
            }

            // other fatal error
            std::cout << B_RED << "[ERROR]: epoll_wait maight be in fatal error, terminating now" << C_RESET << std::endl;
            throw (-1);
        }

        // std::cout << "[DEBUG]: total " << nfds << " are active now" << std::endl;

        // loop each event
        for (int i = 0; i < nfds; i++) {
            
            // set teh context for client to use in read and write state
            HttpHandler* context = NULL;
            if (this->_context.find(events[i].data.fd) != this->_context.end())
                context = this->_context[events[i].data.fd];
        
            // EPOLLIN event
            if (events[i].events & EPOLLIN) {

                // [TODO]: New Connection
                if (this->_fd.find(events[i].data.fd) != this->_fd.end()) {

                    std::cout << S_DEBUG << "New connection request to " << events[i].data.fd << std::endl;

                    int conn = accept(events[i].data.fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_len);
                    if (conn == -1) {
                        std::cout << S_WARNING << "failed to establish connection" << S_END;
                        continue ;
                    }

                    // add to epoll list
                    if (epoll_add(conn, EPOLLIN) == -1) {
                        // remove and close all
                        close(conn);
                        continue ;
                    }

                    // create context instance and add to the context list
                    if (this->_context.find(conn) != this->_context.end()) {
                        std::cout << S_WARNING << "duplicated fd for new connection" << S_END; 
                        epoll_del(conn);
                        close(conn);
                        continue;
                    }

                    this->_context.insert(std::pair<int, HttpHandler*>(conn, new HttpHandler(conn)));
                    
                    std::cout << S_INFO << "connection with " << conn << " is established" << S_END;
                    
                    continue ;
                }

                // [TODO]: reading request event
                if (context != NULL) {

                    // [TODO]: Normal Request
                    if (context->get_status() <= READING_BODY) {

                        std::cout << S_DEBUG << events[i].data.fd << " is in reading state" << S_END;

                        // handle_request(); // this may include the processing state if finish reading
                        
                        // test setting the response
                        context->_res.append("HTTP/1.1 200 OK\r\n");
                        context->_res.append("Content-type: text/plain\r\n");
                        context->_res.append("Content-length: 5\r\n\r\n");
                        context->_res.append("Hello");

                        context->set_status(SENDING); // in case cgi set to cgi state

                        if (context->get_status() == SENDING) {
                            epoll_mod(events[i].data.fd, EPOLLOUT);
                            continue ;
                        }

                        continue;

                    }

                    // [TODO]: for cgi reading
                    if (context->get_status() == CGI_IN) {

                        std::cout << S_DEBUG << events[i].data.fd << " is in [CGI] reading state" << S_END;
                        
                        context->set_status(BUILDING_RES);

                        continue ;                        

                    }

                    continue ;
                }
                

                continue;
            }

            // EPOLLOUT event
            if (events[i].events & EPOLLOUT) {

                // check if it still in the context
                if (context != NULL) {

                    // [TODO]: Sending the response to client
                    if (context->get_status() == SENDING) {

                        std::cout << S_DEBUG << events[i].data.fd << " is in sending state" << S_END;

                        send(events[i].data.fd, context->_res.c_str(), context->_res.size(), 0);

                        context->set_status(COMPLETED);

                        // [TODO]: if connection is keep-alive, clean and return to epollin

                        close_connection(events[i].data.fd);

                        continue ;
                    }
    
                    // [TODO]: sending the input to cgi
                    if (context->get_status() == CGI_OUT) {

                        continue ;
                    }

                }


                continue;
            }

            // EPOLLHUP or EPOLLERR
            if ((events[i].events & EPOLLHUP) || (events[i].events & EPOLLERR)) {

                std::cout << S_ERROR << "epollerr or epollhup" << S_END;
                close_connection(events[i].data.fd);
                
                continue;
            }
            
        }
    }

    std::cout << B_YELLOW << "[INFO]: epoll loop ended" << C_RESET << std::endl;

    return (0);
}

int Webserv::epoll_add(int fd, int op) {
    struct epoll_event ev;
    ev.events = op;
    ev.data.fd = fd;
    if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        epoll_del(fd);
        return (-1);
    }
    return (0);
}

void Webserv::epoll_mod(int fd, int op) {
    struct epoll_event ev;
    ev.events = op;
    ev.data.fd = fd;
    if (epoll_ctl(this->_epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
        std::cout << B_RED << "[ERROR]: epoll_ctl mod failed for " << fd << C_RESET << std::endl;
        throw (-1);
    }
}

void Webserv::epoll_del(int fd) {
    if (epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        std::cout << B_RED << "[ERROR]: epoll_ctl del failed for " << fd << C_RESET << std::endl;
        throw (-1);
    }
}

void Webserv::close_connection(int fd) {

    // clear the context
    if (this->_context.find(fd) != this->_context.end()) {
        delete this->_context[fd];
        this->_context.erase(this->_context.find(fd));
        epoll_del(fd);
        close(fd);
        std::cout << S_WARNING << "connection with " << fd << " was closed" << S_END;
        return ;
    }

    std::cout << S_WARNING << "connection find the " << fd << " in the context list" << S_END;
}
