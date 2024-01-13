#include "Server.hpp"

Server::Server(void) {

}

Server::~Server(void) {

}

// setter
void Server::set_fd(int fd) {
    this->_fd = fd;
};

void Server::add_server_name(std::string name) {
    this->_serverName.push_back(std::string(name));
};

void Server::set_port(int port) {
    this->_port = port;
};

// will received later
void Server::set_default_error_page(std::string str) {

    remove_white_space(str);

    // cut form the backside to get the filename first
    int start, len;

    start = str.rfind(" ");
    len = str.size() - start;
    std::string name(str.substr(start + 1, len - 1));
    str.erase(start, len);

    // std::cout << name << " " << name.size() << std::endl;

    std::stringstream ss(str);
    std::string attr;
    while (std::getline(ss, attr, ' '))
    {
        int code = string_to_int(attr);
        this->_errorCode[code] = std::string(name);
    }
};

void Server::set_max_client_body_size(size_t size) {
    this->_maxClientBodySize = size;
};

void Server::set_root(std::string root) {
    this->_root = std::string(root);
};

void Server::set_allowed_method(std::string method) {
    this->_allowedMethod = std::string(method);
};

void Server::set_location(std::string loc, std::string value) {
    this->_location.insert(std::pair<std::string, std::string>(loc, value));
};

// getter
int Server::get_fd(void) const {
    return (this->_fd);
}

int Server::get_port(void) const {
    return (this->_port);
}

std::string Server::get_root(void) const {
    return (std::string(this->_root));
}

std::string Server::get_server_name(std::string name) const{
    for (int i = 0; i < (int)this->_serverName.size(); i++) {
        if (this->_serverName[i].compare(name) == 0)
            return (std::string(this->_serverName[i]));
    }
    return (std::string(""));
}

int Server::is_server_name_defined(void) const {
    if (this->_serverName.size() == 0)
        return (-1);
    return (0);
}

std::string Server::get_location(std::string loc) {

    if (this->_location.find(loc) != this->_location.end())
        return (std::string(this->_location[loc]));
    return (std::string(""));
}

std::string Server::get_error_code(int code) {
    std::map<int, std::string>::iterator it = this->_errorCode.begin();

    for (; it != this->_errorCode.end(); it++) {
        if (code == it->first) {
            return (std::string(it->second));
        }
    }
    return (std::string(""));

}

size_t Server::get_max_client_body_size(void) const {
    return (this->_maxClientBodySize);
}

// checker
int Server::has_server_name(std::string name) {
    for (int i = 0; i < (int)this->_serverName.size(); i++) {
        // std::cout << this->_serverName[i] << std::endl;
        if (this->_serverName[i].compare(name) == 0) {
            return (1);
        }
    }
    return (0);
}

std::string Server::best_match_location(std::string loc) {

    std::map<std::string, std::string>::iterator it = this->_location.begin();

    int mostMatch = 0;
    std::string output;
    // std::cout << "total size of location: " << this->_location.size() << std::endl;
    for (; it != this->_location.end(); it++) {
        int size = it->first.size();
        // std::cout << it->first << std::endl;
        if (loc.compare(0, it->first.size(), it->first) == 0 && size >= mostMatch) {
            // std::cout << "most match found => " << it->first << std::endl;
            mostMatch = size;
            output = std::string(it->first);
        }
    }

    return (std::string(output));
}

// general
void Server::initiated(int backlog) {

    // create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        std::cout << S_ERROR << "create socket failed" << S_END;
        throw (-1);
    }

    // set socket for reusing address
    int optval = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
        std::cout << S_ERROR << "set socket option failed" << S_END;
        throw (-1);
    }

    // struct sockaddr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;                  // for IPv4
    addr.sin_port = htons(this->get_port());    // set to specific port
    addr.sin_addr.s_addr = htonl(INADDR_ANY);          // for all IP address

    // bind
    if (bind(fd, (struct sockaddr*)&addr, (int)(sizeof(addr))) == -1) {
        std::cout << S_ERROR << "failed to bind port " << this->_port << S_END;
        throw (-1);
    }
    // the COMMON PORT can be binded if we put sudo when starting the server

    // set nonblocking
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        std::cout << S_ERROR << "set sockopt failed" << S_END;
        throw (-1);
    }

    // listen
    if (listen(fd, backlog) == -1) {
        std::cout << S_ERROR << "listen failed" << S_END;
        throw (-1);
    }

    // set socket to it fd
    this->_fd = fd;

};

// Utils
void Server::remove_white_space(std::string &input)
{
    int len = input.size();
    for (int i = 0; i < (int)len; i++)
    {
        if (input[i] == ' ')
        {
            input.erase(i, 1);
            i--;
            len = input.size();
        }
        else
            break;
    }

    // this line was wrong for a while, must check if any output is weird
    for (int i = input.size() - 1; i >= 0; i--)
    {
        if (input[i] == ' ' || input[i] == '\r' || input[i] == '\n')
        {
            input.erase(i, 1);
        }
        else
            break;
    }
}

int Server::string_to_int(std::string str)
{
    std::stringstream ss(str);
    int output;
    ss >> output;
    return (output);
}

void Server::print_debug() {
    std::cout << "[SERVER]fd: " << this->_fd << std::endl;
    std::cout << "[SERVER]port: " << this->_port << std::endl;
    std::cout << "[SERVER]maxClientBodySize: " << this->_maxClientBodySize << std::endl;
    std::cout << "[SERVER]root: " << this->_root << std::endl;
    std::cout << "[SERVER]allowedMethod: " << this->_allowedMethod << std::endl;
    for(size_t i = 0; i < _serverName.size(); i++)
        std::cout << "[SERVER]serverName[ " << i << "]: " << this->_serverName[i] << std::endl;
    for(std::map<int, std::string>::iterator it = _errorCode.begin();
        it != _errorCode.end(); ++it)
        std::cout << "[SERVER]errorCode[" << it->first << "]: " << it->second << std::endl;
    for(std::map<std::string, std::string>::iterator it = _location.begin();
        it != _location.end(); ++it)
        std::cout << "[SERVER]location[" << it->first << "]: " << it->second << std::endl;
    }
    