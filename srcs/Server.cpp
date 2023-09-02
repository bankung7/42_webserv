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
        std::vector<char> bf(BUFFER_SIZE + 1);

        int iread = recv(cfd, bf.data(), BUFFER_SIZE, 0);
        if (iread < 0) {
            std::cout << "[ERROR] : read nothing" << std::endl;
            continue ;
        }

        // parsing
        std::map<std::string, std::string> requestHolder;
        parsing(requestHolder, bf);

        // create the header
        std::string header;
        header.append("HTTP/1.1 200 OK\r\n");
        header.append("Content-type: ");
        header.append(requestHolder["content-type"]);
        header.append("\r\n");
        header.append("Content-Length: ");

        // to get the file info, eg size to be used for sending
        struct stat sb;
        stat(requestHolder["filename"].c_str(), &sb);
        std::cout << "size of file sent : " << sb.st_size << std::endl;

        // append the total size to be sent in body as Content-Length
        std::stringstream iss;
        iss << sb.st_size;

        std::string rsize;
        iss >> rsize;
        header.append(rsize);
        header.append("\r\n\r\n");

        // send header
        send(cfd, (void *)header.c_str(), header.length(), 0);

        int bytesSend = 0;
        int ifile = open(requestHolder["filename"].c_str(), O_RDONLY);
        int rd;
    
        // loop send body
        while (bytesSend < sb.st_size) {
            std::vector<char> bbf(BUFFER_SIZE + 1);
            rd = read(ifile, bf.data(), BUFFER_SIZE);
            if (rd == -1) {
                std::cout << "may be end of file" << std::endl;
                break ;
            }
            send(cfd, (void *)bf.data(), rd, 0);
            bytesSend += rd;
        }

        std::cout << "Total " << bytesSend << " was sent" << std::endl;

        close(rd);
        close(cfd);
        std::cout << std::endl;
    }
}

void Server::parsing(std::map<std::string, std::string> &requestHolder, std::vector<char> request) {
    std::cout << request.data() << std::endl;

    (void)requestHolder;

    std::stringstream ss(request.data());
    std::string arg;
    
    // get method
    std::getline(ss, arg, ' ');
    requestHolder.insert(std::pair<std::string, std::string>("method", arg));
    arg.clear();

    // get resource
    std::getline(ss, arg, ' ');
    requestHolder.insert(std::pair<std::string, std::string>("resource", arg));
    arg.clear();

    // get HTTP version
    std::getline(ss, arg, '\n');
    requestHolder.insert(std::pair<std::string, std::string>("version", arg));

    // set content-type
    if (requestHolder["resource"].length() == 1) {
        requestHolder.insert(std::pair<std::string, std::string>("Content-type", "text/html"));
        requestHolder.insert(std::pair<std::string, std::string>("filename", "sites/index.html"));
        return ;
    } 
    if (requestHolder["resource"].compare("/panda.jpg") == 0) {
        requestHolder.insert(std::pair<std::string, std::string>("Content-type", "image/jpeg"));
        requestHolder.insert(std::pair<std::string, std::string>("filename", "sites/panda.jpeg"));
        return ;
    }

    // other case handle later
    requestHolder.insert(std::pair<std::string, std::string>("Content-type", "text/html"));
    requestHolder.insert(std::pair<std::string, std::string>("filename", "sites/404.html"));

}

// void Server::createHeader(std::string &header) {
//     header.append("HTTP/1.1 200 OK\r\n");
// }