#include "HttpHandler.hpp"

HttpHandler::HttpHandler(void) {

}

HttpHandler::HttpHandler(int fd): _fd(fd), _status(READING) {

    this->_parameter["Host"] = std::string("");
    this->_parameter["Connection"] = std::string("");
}

HttpHandler::~HttpHandler(void) {

}

// setter
void HttpHandler::set_status(int status) {
    this->_status = status;
}

void HttpHandler::set_server(std::vector<Server>& server) {
    this->_server = server;
}

// getter
int HttpHandler::get_fd(void) const {
    return (this->_fd);
}

int HttpHandler::get_status(void) const {
    return (this->_status);
}

// process
void HttpHandler::handle_request(void) {

    // try to read the request with buffer
    std::vector<char> bf(BUFFER_SIZE);
    int bytesRead;

    // reading in 1 loop
    // seem like when the connection was close and epoll does not pick that as a event
    while (1) {
        bytesRead = recv(this->get_fd(), bf.data(), BUFFER_SIZE, 0);
        if (bytesRead <= 0) {
            std::cout << "[ERROR]: The connection was closed or error occured" << std::endl;
            break;
        }
        this->_req.append(bf.data(), bytesRead);
    }

    std::cout << "[DEBUG]: reading incoming " << bytesRead << std::endl;
    this->set_status(WRITING);

    std::cout << this->_req << std::endl;

    this->parsing_request();
}

void HttpHandler::parsing_request(void) {

    std::stringstream ss(this->_req);
    std::string line;

    // get status line
    std::getline(ss, line, '\n');
    std::stringstream lss(line);
    std::getline(lss, this->_method, ' ');
    std::getline(lss, this->_url, ' ');
    std::getline(lss, this->_version);

    // loop for other attribute
    while (std::getline(ss, line, '\n')) {
        std::stringstream lss(line);
        std::string attr;
        std::getline(lss, attr, ':');
        if (this->_parameter.find(attr) != this->_parameter.end()) {
            std::string value;
            std::getline(lss, value);
            this->remove_white_space(value);
            this->_parameter[attr] = std::string(value);
        }
    }

    std::cout << "Method: " << this->_method << std::endl;
    std::cout << "URL: " << this->_url << std::endl;
    std::cout << "Version: " << this->_version << std::endl;
    std::cout << "Host: " << this->_parameter["Host"] << std::endl;
    std::cout << "Connection: " << this->_parameter["Connection"] << std::endl;

}

void HttpHandler::handle_response(void) {

    // TODO: case of only hostname appear

    // split and set hostname and port
    std::string sport(this->_parameter["Host"]);
    sport.erase(0, sport.find(":") + 1);

    this->_parameter["Host"].erase(this->_parameter["Host"].find(sport) - 1, sport.size() + 1);

    std::stringstream ss(sport);
    ss >> this->_port;

    std::cout << "host: " << this->_parameter["Host"] << std::endl;
    std::cout << "port: " << this->_port << std::endl;

    // std::stringstream cport(this->_host.substr(this->_host.find(":"), this->_host.size() - this->_host.find(":")));
    // std::cout << cport.str() << std::endl;

    // match host and assign server
    // std::vector<Server>::iterator it = this->_server.begin();
    // for (; it != this->_server.end(); it++) {
    //     if (this->_server[i].get_port() != )
    // }

    // match location
}


// Utils
void HttpHandler::remove_white_space(std::string& input) {
    int len = input.size();
    for (int i = 0; i < (int)len; i++) {
        if (input[i] == ' ') {
            input.erase(i, 1);
            i--;
        }
        else
            break;
    }
}