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
        std::string request;
        request.append(std::string(bf));
        free(bf);

        // std::cout << request << std::endl;

        // check the header request
        std::stringstream ss(request);
        std::string fline;
        std::getline(ss, fline, '\n');

        std::stringstream ss2(fline);
        std::string rfile;
        std::getline(ss2, rfile, ' ');
        std::getline(ss2, rfile, ' ');

        // just try to catch the request path or file
        std::cout << fline << std::endl;
        std::cout << rfile << std::endl;

        // create the header
        std::string header = "HTTP/1.1 200 OK\r\n";

        std::string fileName;
        if (rfile.length() == 1) {
            header.append("Content-type: text/html\r\n");
            fileName.append("sites/index.html");
            std::cout << "==> Index file" << std::endl;
        }
        else if (rfile.compare("/panda.jpg") == 0) {
            header.append("Content-type: image/jpeg\r\n");
            fileName.append("sites/panda.jpeg");
            std::cout << "==> Panda file" << std::endl;
        }
        else {
            header.append("Content-type: text/html\r\n");
            fileName.append("sites/index.html");
            std::cout << "==> File not found" << std::endl;
        }
        header.append("Content-Length: ");

        std::cout << "file request : " << fileName << std::endl;

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
        
        close(rd);
        close(cfd);
        std::cout << std::endl;
    }
}