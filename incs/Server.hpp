#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.h"

class DataHolder;

class Server {

private:
    int _sfd;
    struct sockaddr_in _saddr;
    int _saddrLen;

public:
    Server(void);
    ~Server(void);

    void start(void);
    void setConfigFile(std::string file);

};

#endif