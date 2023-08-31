#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.h"

class Server {

private:
    int _sfd;
    sockaddr_in _saddr;
    int _saddrLen;
    std::map<std::string, std::string> _requestData;

    void start(void);
    int parsing(std::string req);
    int getRequestLine(std::string line);
    void readRequest(void);

public:
    Server(void);
    ~Server(void);


};

#endif