#include "Webserv.hpp"

int Webserv::polling(void)
{
    // create epoll instance
    this->_epfd = epoll_create(1);
    if (this->_epfd == -1)
    {
        std::cout << S_ERROR << "epoll_create() failed for some reason" << S_END;
        throw(-1);
    }

    // add all server listener to epoll list
    struct epoll_event ev;
    for (std::set<int>::iterator it = this->_fd.begin(); it != this->_fd.end(); it++)
    {
        ev.events = EPOLLIN;
        ev.data.fd = *it;
        if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, *it, &ev) == -1)
        {
            std::cout << S_ERROR << "failed to add listener to epoll list" << S_END;
            throw(-1);
        }
        std::cout << S_GREEN << "[" << *it << "] is on the epoll list" << S_END;
    }

    std::cout << S_DEBUG << "Starting Epoll loop now" << S_END;

    // create variable
    struct epoll_event events[MAX_EVENTS];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int nfds;

    while (1)
    {

        nfds = epoll_wait(this->_epfd, events, MAX_EVENTS, 1000);

        // check that time out
        check_time_out();

        // if wait error
        if (nfds == -1)
        {
            if (errno == EINTR)
            {
                std::cout << S_WARNING << "epoll_wait was interupted for some reason, will procedd in next loop" << S_END;
                continue;
            }

            // other fatal error
            std::cout << S_ERROR << "epoll_wait maight be in fatal error, terminating now" << S_END;
            throw(-1);
        }

        // std::cout << "[DEBUG]: total " << nfds << " are active now" << std::endl;

        // std::cout << "context size: " << this->_context.size() << std::endl;

        // loop each event
        for (int i = 0; i < nfds; i++)
        {

            // set teh context for client to use in read and write state
            HttpHandler *context = NULL;
            if (this->_context.find(events[i].data.fd) != this->_context.end())
                context = this->_context[events[i].data.fd];

            // EPOLLIN event
            if (events[i].events & EPOLLIN)
            {

                // [TODO]: New Connection
                if (this->_fd.find(events[i].data.fd) != this->_fd.end())
                {

                    std::cout << S_DEBUG << "[" << events[i].data.fd << "] request to connect" << S_END;

                    int conn = accept(events[i].data.fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
                    if (conn == -1)
                    {
                        std::cout << S_WARNING << "[" << events[i].data.fd << "] failed to establish connection" << S_END;
                        continue;
                    }

                    // set nonblocking
                    if (fcntl(conn, F_SETFL, O_NONBLOCK) == -1)
                    {
                        std::cout << S_WARNING << "[" << events[i].data.fd << "] set non block failed" << S_END;
                        close(conn);
                        continue;
                    }

                    // add to epoll list
                    if (epoll_add(conn, EPOLLIN) == -1)
                    {
                        // remove and close all
                        close(conn);
                        continue;
                    }

                    // create context instance and add to the context list
                    if (this->_context.find(conn) != this->_context.end())
                    {
                        std::cout << S_WARNING << "[" << events[i].data.fd << "] duplicated fd for new connection" << S_END;
                        epoll_del(conn);
                        close(conn);
                        continue;
                    }

                    this->_context.insert(std::pair<int, HttpHandler *>(conn, new HttpHandler(conn, this->_server)));

                    std::cout << S_GREEN << "[" << conn << "] connection is established" << S_END;

                    continue;
                }
                // ================================>

                // [TODO]: reading request event
                if (context != NULL)
                {

                    // [TODO]: for cgi reading
                    if (context->get_status() == CGI_IN)
                    {
                        // std::cout << S_DEBUG << "[" << events[i].data.fd << "] [CGI] reading state" << S_END;
                        context->cgi_reading();

                        if (context->cgi_get_status() == ERROR)
                        {
                            epoll_del(context->cgi_get_from_fd());
                            this->_cgiList.erase(context->cgi_get_from_fd());
                            this->_context.erase(context->cgi_get_from_fd());
                            close(context->cgi_get_from_fd());
                        }

                        if (context->get_status() == CONTENT_PHASE)
                        {
                            context->content_builder();
                            epoll_mod(context->get_fd(), EPOLLOUT);
                            // std::cout << S_INFO << "content completed, send to epoll out for seding" << S_END;
                            continue;
                        }

                        continue;
                    }
                    // ==========================>

                    // [TODO]: Normal Request
                    if (context->get_status() <= READING_BODY)
                    {

                        // std::cout << S_DEBUG << events[i].data.fd << " is in reading state" << S_END;

                        // this may include the processing state if finish reading
                        context->handle_request();

                        // if there is a cgi job to run
                        if (context->get_status() == CGI_OUT)
                        {
                            // std::cout << S_INFO << "push to cgi writing step" << S_END;
                            // epoll_mod(context->get_fd(), EPOLLOUT);

                            // add both pipe to the epoll loop
                            epoll_add(context->cgi_get_to_fd(), EPOLLOUT);
                            this->_context[context->cgi_get_to_fd()] = context;
                            this->_cgiList[context->cgi_get_from_fd()] = context->get_fd();

                            epoll_add(context->cgi_get_from_fd(), EPOLLIN);
                            this->_context[context->cgi_get_from_fd()] = context;
                            this->_cgiList[context->cgi_get_to_fd()] = context->get_fd();

                            continue;
                        }

                        // if all thing built, sent to epollout
                        if (context->get_status() == SENDING)
                        {
                            epoll_mod(context->get_fd(), EPOLLOUT);
                            continue;
                        }

                        // if the connection was closed or error happen
                        if (context->get_status() == CLOSED)
                        {
                            close_connection(context->get_fd());
                            continue;
                        }

                        continue;
                    }
                    // ==========================>

                    continue;
                }

                continue;
            }
            // ================================>

            // EPOLLOUT event
            if (events[i].events & EPOLLOUT)
            {

                // check if it still in the context
                if (context != NULL)
                {

                    // [TODO]: CGI Writing
                    if (context->get_status() == CGI_OUT)
                    {

                        // std::cout << S_INFO << "[" << context->get_fd() << "][CGI][" << events[i].data.fd << "] writing on process" << S_END;

                        context->cgi_writing();

                        if (context->get_status() == CGI_IN)
                        {

                            std::cout << S_INFO << "[" << context->get_fd() << "][CGI][" << context->cgi_get_to_fd() << "] writing completed, push to CGI reading" << S_END;

                            epoll_del(context->cgi_get_to_fd()); // remove itself
                            this->_context.erase(this->_context.find(context->cgi_get_to_fd()));
                            this->_cgiList.erase(context->cgi_get_to_fd());
                            close(context->cgi_get_to_fd());

                            std::cout << S_INFO << "[" << context->get_fd() << "][CGI][" << context->cgi_get_to_fd() << "] removing the to_cgi_fd from the list" << S_END;

                            continue;
                        }

                        std::cout << S_INFO << "[" << context->get_fd() << "][CGI] still have thing to write" << S_END;
                        continue;
                    }

                    // [TODO]: Sending the response to client
                    if (context->get_status() == SENDING)
                    {

                        // std::cout << S_DEBUG << events[i].data.fd << " is in sending state" << S_END;
                        context->sending();

                        // [TODO]: if connection is keep-alive, clean and return to epollin

                        if (context->get_status() == CLOSED)
                        {
                            close_connection(events[i].data.fd);
                            continue;
                        }

                        if (context->get_status() == COMPLETED_PHASE)
                        {
                            if (context->get_connection_type().find("closed") == 0)
                            {
                                close_connection(events[i].data.fd);
                                continue;
                            }

                            // for keep alive let it closed by time out, reset it first
                            if (this->_context.find(events[i].data.fd) != this->_context.end() && this->_context[events[i].data.fd] != NULL)
                            {
                                delete this->_context[events[i].data.fd];
                                this->_context[events[i].data.fd] = NULL;
                            }

                            this->_context[events[i].data.fd] = new HttpHandler(events[i].data.fd, this->_server);
                            epoll_mod(events[i].data.fd, EPOLLIN);
                            // std::cout << "keep alive, go to in state again" << std::endl;
                            continue;
                        }

                        continue;
                    }
                }

                continue;
            }
            // =======================>

            // EPOLLHUP or EPOLLERR
            if ((events[i].events & EPOLLHUP))
            {
                std::cout << S_INFO << "[" << events[i].data.fd << "] EPOLLHUP" << S_END;
                if (context->get_status() == CGI_IN)
                {

                    this->_context.erase(this->_context.find(context->cgi_get_from_fd()));
                    this->_cgiList.erase(context->cgi_get_from_fd());

                    epoll_del(context->cgi_get_from_fd()); // remove it from the list
                    close(context->cgi_get_from_fd());

                    std::cout << S_DEBUG << "[" << events[i].data.fd << "] Reading output from CGI Completed" << S_END;

                    context->set_status(CONTENT_PHASE);
                    context->content_builder();
                    epoll_mod(context->get_fd(), EPOLLOUT);

                    continue;
                }
                else
                {
                    std::cout << S_ERROR << "[" << events[i].data.fd << "] epollhup" << S_END;
                    close_connection(events[i].data.fd);
                    continue;
                }
            }
            // =======================>

            // For error case
            if (events[i].events & EPOLLERR)
            {
                std::cout << S_ERROR << "[" << events[i].data.fd << "] epollerror" << S_END;
                close_connection(events[i].data.fd);
                continue;
            }
            // =======================>
        }
    }

    std::cout << S_INFO << "epoll loop ended" << S_END;

    return (0);
}

int Webserv::epoll_add(int fd, int op)
{
    struct epoll_event ev;
    ev.events = op;
    ev.data.fd = fd;
    if (epoll_ctl(this->_epfd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        std::cout << S_ERROR << "[" << fd << "] epoll_ctl add " << fd << " failed" << S_END;
        return (-1);
    }
    return (0);
}

void Webserv::epoll_mod(int fd, int op)
{
    struct epoll_event ev;
    ev.events = op;
    ev.data.fd = fd;
    if (epoll_ctl(this->_epfd, EPOLL_CTL_MOD, fd, &ev) == -1)
    {
        std::cout << S_ERROR << "[" << fd << "] epoll_ctl mod failed for " << fd << S_END;
        throw(-1);
    }
}

void Webserv::epoll_del(int fd)
{
    if (epoll_ctl(this->_epfd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        std::cout << S_ERROR << "[" << fd << "] epoll_ctl del failed for " << fd << S_END;
        throw(-1);
    }
}

void Webserv::close_connection(int fd)
{

    // clear the context
    if (this->_context.find(fd) != this->_context.end())
    {
        if (this->_context[fd] != NULL)
        {
            delete this->_context[fd];
            this->_context[fd] = NULL;
        }
        this->_context.erase(this->_context.find(fd));
        epoll_del(fd);
        close(fd);
        std::cout << S_WARNING << "[" << fd << "] connection was closed" << S_END;
        return;
    }

    std::cout << S_WARNING << "[" << fd << "]connection find in the context list" << S_END;
}

void Webserv::check_time_out(void)
{

    std::time_t ctime;
    time(&ctime);

    std::vector<int> fd_list;
    std::vector<int> cgi_list;

    std::map<int, HttpHandler *>::iterator it = this->_context.begin();

    // std::cout << this->_context.size() << "/" << this->_cgiList.size() << std::endl;

    // check and put the timeout into the list, then clear it below.
    for (; it != this->_context.end(); it++)
    {
        // std::cout << "Checking timeout ";
        if (this->_cgiList.find(it->first) != this->_cgiList.end())
        {
            // std::cout << S_WARNING << "[CGI] " << it->first << " = " << this->_cgiList[it->first] << S_END;
            continue;
        }
        else
        {
            // std::cout << "client" << it->first << S_END;
        }

        HttpHandler *cont = it->second;

        // std::cout << S_DEBUG << "client [" << cont->get_fd() << "] : status [" << cont->get_status() << "]";
        // std::cout << " time out in " << cont->get_time_out() - ctime << S_END;

        if (cont != NULL && cont->get_status() <= SENDING && cont->get_time_out() < ctime)
        {
            // client fd itself
            if (cont != NULL && cont->get_fd() == it->first)
            {

                std::cout << S_WARNING << "[" << cont->get_fd() << "] client has time out state=[]" << S_END;

                if (std::find(fd_list.begin(), fd_list.end(), it->first) == fd_list.end())
                {

                    if (cont != NULL && cont->get_status() == CGI_IN)
                    {
                        std::cout << S_INFO << "[" << cont->get_fd() << "] client stuck at CGI IN, kill it" << S_END;
                        kill(cont->cgi_get_pid(), SIGINT); // kill it
                        cont->set_timeout(2);

                        // should kill the stuck cgi fd
                        epoll_del(cont->cgi_get_from_fd());
                        this->_cgiList.erase(cont->cgi_get_from_fd());
                        this->_context.erase(cont->cgi_get_from_fd());
                        close(cont->cgi_get_from_fd());

                        cont->set_status(CONTENT_PHASE);
                        cont->content_builder();
                        epoll_mod(cont->get_fd(), EPOLLOUT);

                        continue;
                    }
                    else
                    {
                        // kick out for one who reading only, maybe
                        fd_list.push_back(it->first);
                    }
                }
            }
            // CGI fd
            else
            {
                std::cout << S_WARNING << "[" << cont->get_fd() << "][CGI FD] =[" << it->first << "]" << S_END;
                cgi_list.push_back(it->first);
            }
        }
    }

    // remove cgi_fd from the loop
    for (int i = 0; i < (int)cgi_list.size(); i++)
    {
        std::cout << "[CGI] " << cgi_list[i] << " will be removed from epoll" << S_END;
        epoll_del(cgi_list[i]);
        this->_context.erase(cgi_list[i]);
    }

    // loop erase that expired, and remove it
    for (int i = 0; i < (int)fd_list.size(); i++)
    {
        close_connection(fd_list[i]);
    }
}
