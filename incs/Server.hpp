#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.h"

class DataHolder;

class Server {

private:
    int _sfd;
    sockaddr_in _saddr;
    int _saddrLen; // may be create another class with their request data

    void start(void);
    // void parsing(std::map<std::string, std::string> &requestHolder, std::vector<char>);
    void parsing(DataHolder &holder, std::vector<char>);
    void getRequest(std::string &header);
    void setResponse(void);


public:
    Server(void);
    ~Server(void);

};

#endif