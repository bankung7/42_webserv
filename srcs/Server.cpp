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

    if (setsockopt(this->_sfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&this->_saddr, this->_saddrLen) < 0)
        throw std::runtime_error("[ERROR] : set socket address reuse failed");

    if (bind(this->_sfd, (sockaddr *)&this->_saddr, this->_saddrLen) != 0)
        throw std::runtime_error("[ERROR] : (bind failed)");
    std::cout << "[DEBUG] : bind was successfully" << std::endl;

    // listen
    if (listen(this->_sfd, SOMAXCONN) != 0)
        throw std::runtime_error("[ERROR] : listen failed");
    std::cout << "[DEBUG] : listen is ready" << std::endl;

    // try connect with incoming
    while (1) {

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
        std::cout << bf << std::endl;
        free(bf);

        // send response
        char header[] = "HTTP/1.0 200 OK\r\n"
                  "Server: webserver-c\r\n"
                  "Content-type: text/html\r\n\r\n";
        char *res = (char *)malloc(sizeof(char) * BUFFER_SIZE);
        memcpy((void *)res, (const void *)header, strlen(header));

        // open file to send
        int file = open("sites/index.html", O_RDONLY);
        char *ifile = (char *)malloc(sizeof(char) * (BUFFER_SIZE - strlen(header) - 3));
        while (read(file, ifile, (BUFFER_SIZE - strlen(header) - 3)) > 0) {
            ifile[(BUFFER_SIZE - strlen(header) - 3)] = 0;
            memcpy((void *)&res[strlen(res)], (const void *)ifile, strlen(ifile));

            int owrite = send(cfd, res, strlen(res), 0);
            if (owrite < 0) {
                std::cout << "[ERROR] : response failed" << std::endl;
                free(res);
                free(ifile);
                continue ;
            }
            std::cout << "[DEBUG] : response has been sent to client [" << cfd << "]" << std::endl;
            std::cout << res << std::endl;
        }
        free(res);

        close(cfd);
    }
}