#include "Server.hpp"

Server::Server(void) {

    // start
    try {
        start();
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

}

Server::~Server(void) {

}

void Server::start(void) {

    // create server socket
    this->_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_sfd == -1)
        throw std::runtime_error("[ERROR] : (socket failed)");
    std::cout << "[DEBUG] : socket has been created successfully" << std::endl;

    // bind ip and port
    this->_saddr.sin_family = AF_INET;
    this->_saddr.sin_port = htons(PORT);
    this->_saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    this->_saddrLen = sizeof(this->_saddr);

    // set to res use the address, prevent waiting for old address killed in last process
    if (setsockopt(this->_sfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&this->_saddr, this->_saddrLen) < 0)
        throw std::runtime_error("[ERROR] : set socket address reuse failed");

    // bind socket with port
    if (bind(this->_sfd, (sockaddr *)&this->_saddr, this->_saddrLen) != 0)
        throw std::runtime_error("[ERROR] : (bind failed)");
    std::cout << "[DEBUG] : bind was successfully" << std::endl;

    // listen
    if (listen(this->_sfd, SOMAXCONN) != 0)
        throw std::runtime_error("[ERROR] : listen failed");
    std::cout << "[DEBUG] : listen is ready" << std::endl;

    // try connect with incoming
    while (1) {

        // accept the request as fd
        int cfd = accept(this->_sfd, (sockaddr *)&this->_saddr, (socklen_t *)&this->_saddrLen);
        if (cfd < 0) {
            std::cout << "[ERROR] : accept failed to the connection" << std::endl;
            continue ;
        }
        std::cout << "[DEBUG] : connection [" << cfd << "] was accetped" << std::endl;

        // read incoming request
        char *bf = (char *)malloc(sizeof(char) * BUFFER_SIZE + 1);
        int iread = recv(cfd, bf, BUFFER_SIZE, 0);
        bf[iread] = 0;
        if (iread < 0) {
            std::cout << "[ERROR] : read nothing" << std::endl;
            continue ;
        }
        std::string request(bf);
        free(bf);

        // std::cout << request << std::endl;

        // parsing
        parsing(request);

        // create the header
        std::string header = "HTTP/1.1 200 OK\r\n";

        std::string fileName("sites");
        if (this->_requestData["Target"].length() == 1) {
            header.append("Content-type: text/html\r\n");
            fileName.append("/index.html");
        }
        else if (this->_requestData["Target"].compare("/panda.jpg") == 0) {
            header.append("Content-type: image/jpeg\r\n");
            fileName.append("/panda.jpeg");
        }
        else {
            header.append("Content-type: text/html\r\n");
            fileName.append("/404.html");
        }
        header.append("Content-Length: ");

        // to get the file info, eg size to be used for sending
        struct stat sb;
        stat(fileName.c_str(), &sb);
        std::cout << "size of file sent : " << sb.st_size << std::endl;

        // append the total size to be sent in body as Content-Length
        header.append(std::to_string(sb.st_size));
        header.append("\r\n\r\n");

        // send header
        send(cfd, (void *)header.c_str(), header.length(), 0);

        int bytesSend = 0;
        int ifile = open(fileName.c_str(), O_RDONLY);
        int rd;
    
        // loop send body
        while (bytesSend < sb.st_size) {
            bf = (char *)malloc(sizeof(char) * (BUFFER_SIZE + 1));
            rd = read(ifile, bf, BUFFER_SIZE);
            if (rd == -1) {
                std::cout << "may be end of file" << std::endl;
                free(bf);
                break ;
            }
            bf[rd] = 0;
            send(cfd, (void *)bf, rd, 0);
            free(bf);
            bytesSend += rd;
        }

        std::cout << "Total " << bytesSend << " was sent" << std::endl;

        this->_requestData.clear();
        close(rd);
        close(cfd);
        std::cout << std::endl;
    }
}

int Server::parsing(std::string req) {

    if (req.empty())
        return (1);

    std::stringstream ss(req);
    std::string line;

    // get request line
    std::getline(ss, line, '\n');

    getRequestLine(line);

    // loop the rest
    while (std::getline(ss, line, '\n')) {
        if (line.empty())
            break ;
        std::stringstream iss(line);
        std::string arg;
        std::string val;
        std::getline(iss, arg, ':');
        std::getline(iss, val);
        if (arg.empty() == 1 || val.empty() == 1)
            continue ;
        this->_requestData.insert(this->_requestData.end(), std::pair<std::string, std::string>(arg, val));
    }

    readRequest();

    return (0);
}

int Server::getRequestLine(std::string line) {

    if (line.empty())
        return (1);
    
    std::stringstream ss(line);
    std::string input;

    // get method [GET POST DELETE]
    std::getline(ss, input, ' ');
    if (input.empty())
        return (1);
    this->_requestData.insert(this->_requestData.end(), std::pair<std::string, std::string>("Method", input));

    // get target
    std::getline(ss, input, ' ');
    if (input.empty())
        return (1);
    this->_requestData.insert(this->_requestData.end(), std::pair<std::string, std::string>("Target", input));

    // get HTTP version
    std::getline(ss, input, '\n');
    if (input.empty())
        return (1);
    this->_requestData.insert(this->_requestData.end(), std::pair<std::string, std::string>("HTTP version", input));

    return (0);
}

void Server::readRequest(void) {
    std::map<std::string, std::string>::iterator it;

    std::cout << "==================================" << std::endl;
    it = this->_requestData.begin();
    for (; it != this->_requestData.end(); it++) {
        std::cout << it->first << " : " << it->second << std::endl;
    }
    std::cout << "==================================" << std::endl;
}