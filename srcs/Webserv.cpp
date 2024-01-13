#include "Webserv.hpp"

static void signal_handler(int sig)
{
    (void)sig;
    signal(SIGINT, SIG_IGN);
    throw(0);
}

Webserv::Webserv(void) : _backlog(BACKLOG)
{
    starting(std::string(""));
}

Webserv::Webserv(std::string filename) : _backlog(BACKLOG)
{
    starting(filename);
}

void Webserv::starting(std::string filename)
{
    std::cout << B_GREEN << "Webserv initiated" << S_END;

    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN); // broken pipe when siege

    // for main loop
    try
    {
        if (filename.size() == 0) {
            Conf cf;
            cf.parseconf(this->_server);
        }
        else {
            Conf cf(filename);
            cf.parseconf(this->_server);
        }
        std::cout << "[SOUND]: " << this->_server[0].get_max_client_body_size() << std::endl;
        setup();
        polling();

    }
    catch (std::exception &e)
    {
        std::cout << S_ERROR << e.what() << S_END;
    }
    catch (int e)
    {
        switch (e)
        {
        case (0):
            std::cout << S_WARNING << "Shutdown signal detected" << S_END;
            break;
        case (-1):
            std::cout << S_ERROR << "Something wrong happed" << S_END;
            break;
        default:
            break;
        }
    }

    std::cout << S_WARNING << "Prepare closing the server" << S_END;
    // ~Webserv();

}

Webserv::~Webserv(void)
{
    // should use destructor ???

    // cleaning
    try
    {
        clean_context();
        clean_socket();
        close(this->_epfd);
    } catch (...) {

    }

    std::cout << B_GREEN << "[INFO]: Exit completed" << S_END;

}

// Server Structure Setup Part
void Webserv::setup(void)
{

    std::cout << S_DEBUG << "Setting up the server" << S_END;

    // loop read the _server to initiated
    for (int i = 0; i < (int)this->_server.size(); i++)
    {

        int port = this->_server[i].get_port();

        if (this->_port.find(port) == this->_port.end())
        {

            this->_server[i].initiated(BACKLOG);
            this->add_fd(this->_server[i].get_fd());
            this->add_port(port);
            std::cout << S_DEBUG << "Create socket [" << this->_server[i].get_fd() << "] for port [" << port << "]" << S_END;
            continue;
        }

        // Port Duplication case
        this->_log.clear();
        this->_log << S_ERROR << "Port [" << port << "] is duplicatied" << S_END;
        throw std::runtime_error(this->_log.str());
    }
};

// setter
void Webserv::add_fd(int fd)
{
    this->_fd.insert(fd);
}

void Webserv::add_server(Server sv)
{
    this->_server.push_back(sv);
}

void Webserv::add_port(int port)
{
    this->_port.insert(port);
}

void Webserv::add_context(int fd, HttpHandler *context)
{

    if (this->_context.find(fd) != this->_context.end())
        throw std::runtime_error("duplicated fd in context");

    this->_context[fd] = context;
}

// remover
void Webserv::remove_context(int fd)
{

    std::map<int, HttpHandler *>::iterator it = this->_context.find(fd);

    if (it != this->_context.end())
        this->_context.erase(it);
}

// error handling part
void Webserv::clean_socket(void)
{

    std::set<int>::iterator it = this->_fd.begin();

    for (; it != this->_fd.end(); it++)
    {
        std::cout << S_WARNING << "server socket " << *it << " is being closed now" << S_END;
        epoll_del(*it); // remove it from epoll events
        close(*it);
    }

    this->_fd.clear();
}

void Webserv::clean_context(void)
{

    std::map<int, HttpHandler *>::iterator it = this->_context.begin();

    std::cout << this->_context.size() << std::endl;

    for (; it != this->_context.end(); it++)
    {
        std::cout << S_INFO << "Context for client " << it->first << " is being deleted" << S_END;
    
        epoll_del(it->first); // remove it from epoll events
        close(it->first);     // close client socket

        if (this->_cgiList.find(it->first) == this->_cgiList.end()) {
            if (it->second != NULL) {
                delete it->second;    // delete the client context
                it->second = NULL;
            }
        }

        // std::cout << "delete context for fd " << it->first << S_END;
    }

    // this->_context.clear();
}
