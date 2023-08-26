#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.h"

class Server {

private:
    int _sfd;
    sockaddr_in _saddr;
    int _saddrLen;
    void start(void);
    void sendJPG(int fd, std::string name);

public:
    Server(void);
    ~Server(void);


};

#endif