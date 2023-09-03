#ifndef DATAHOLDER_HPP
#define DATAHOLDER_HPP

#include "webserv.h"

class DataHolder {

private:
    void parsing(std::vector<char> &);
    void getRequest(void);
    void processRequest(void);
    void setResponse(void);

public:
    DataHolder(void);
    DataHolder(std::vector<char> &);
    ~DataHolder(void);


    // request holder
    std::string _method;
    std::string _resource;
    std::string _cversion;

    // general part
    std::string _filename;

    // response holder
    std::string _header;
    int _headerLength;
    std::string _sversion;
    std::string _statusCode;
    std::string _reason;
    std::string _contentType;
    std::string _contentLength;
    int _bodyLength;

};

#endif
