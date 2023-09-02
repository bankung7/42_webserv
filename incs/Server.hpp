#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.h"

class Server {

private:
    int _sfd;
    sockaddr_in _saddr;
    int _saddrLen; // may be create another class with their request data

    void start(void);
    void parsing(std::map<std::string, std::string> &requestHolder, std::vector<char>);
    // void createHeader(std::string &header);


public:
    Server(void);
    ~Server(void);

};

#endif