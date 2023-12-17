#include "HttpHandler.hpp"

// ignore default constructor
HttpHandler::HttpHandler(void) {}


HttpHandler::HttpHandler(int fd): _fd(fd) {

}

HttpHandler::~HttpHandler(void) {

}

// https://nginx.org/en/docs/http/request_processing.html
// https://nginx.org/en/docs/dev/development_guide.html#http_request

// ============================ REQUEST PART - [START] =================================

// request handling part
void HttpHandler::handlingRequest(void) {

    // 1- readind form fd
    int rd;
    char bf[BUFFER_SIZE];
    std::stringstream ss;

    ss << std::noskipws;

    do {
        rd = recv(this->_fd, bf, BUFFER_SIZE, 0);
        if (rd < 1024)
            bf[rd] = 0;

        // TODO: error handling
        if (rd <= 0)
            break ;

        ss << bf;
    } while (rd > 0);

    // save to req
    this->_req = std::string(ss.str());

    std::cout << this->_req << std::endl;

    check_host();

}

// check server_name to assign correct server block
int HttpHandler::check_host(void) {

    // loop read line to check the host
    std::stringstream ss(this->_req);
    std::string line;

    while (std::getline(ss, line, '\n')) {

        if (line.compare(0, 6, "Host: ") == 0) {
            std::string host(line);
            std::string strport;
            host.erase(0, 6);
            size_t sep = host.find(':');
            if (sep != std::string::npos) {
                strport = std::string(host, sep + 1, host.size() - sep - 1);
                host.erase(sep, host.size() - sep);
            }
            this->_host = std::string(host);
            std::cout << "this host is " << this->_host << " : " << strport << std::endl;

            // convert
            std::stringstream iss(strport);
            int port;
            iss >> port;

            // loop check the server_name in this port
            this->_server_index = -1;
            for (int i = 0; i < (int)this->_serverList.size(); i++) {
                if (this->_serverList[i].get_port() == port) {
                    std::cout << "same port " << port << std::endl;
                    int index = this->_serverList[i].check_server_name(host);
                    if (index != -1) {
                        std::cout << "index is " << i << std::endl;
                        this->_server_index = i;
                        break ;
                    }
                }
            }

            // if not found, set the to default server
            if (this->_server_index == -1)
                this->_server_index = 0;
            
            // set the server
            this->set_server(this->_serverList[this->_server_index]);

            break ;
        }

    }

    return (0);
}

// ============================ REQUEST PART - [END] =================================

// ============================ RESPONSE PART - [START] =================================

// response handling
void HttpHandler::handlingResponse(void) {

    // =========================  Open the file =============================
        
    // TODO: create response
    Server sv = this->_server;

    // get file name to send
    std::string fileName(sv.get_root());
    fileName.append(std::string("html/index.html"));
    fileName.erase(fileName.begin());

    // std::cout << "try to send the file name [" << fileName << "]" << std::endl;

    std::ifstream file;
    file.open(fileName.c_str(), std::ios::in);

    if (!file)
        throw std::runtime_error("[ERROR]: cannot open the file");

    file.seekg(0, std::ios::end);

    int fileSize = file.tellg();

    file.seekg(0, std::ios::beg);

    // ================  create and send response header  ====================

    std::stringstream ss;
    ss << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: text/html\r\n"
        // << "Content-Length: " << this->_req.size() << "\r\n\r\n"
        << "Content-Length: " << fileSize << "\r\n\r\n";

    // send the header
    std::string rHeader = std::string(ss.str());
    std::vector<char> header;
    header.insert(header.begin(), rHeader.begin(), rHeader.end());
    send(this->_fd, header.data(), header.size(), MSG_NOSIGNAL);

    // ==================  create and send response body  =====================

    std::stringstream body;
    body << file.rdbuf();
    std::string res = body.str();
    std::vector<char> msg;
    msg.insert(msg.begin(), res.begin(), res.end());

    // loop send body
    int totalBytes = fileSize;
    int bytes = 0;

    while (bytes < totalBytes) {
        int sentBytes = send(this->_fd, msg.data() + bytes, BUFFER_SIZE, MSG_NOSIGNAL);

        // TODO: error handling
        if (sentBytes <= 0) {
            std::cout << "[ERROR]: sending failed " << this->_fd << std::endl;
            break ;
        }

        bytes += sentBytes;
    }

    // ==========================================================================

    std::cout << "[DEUBG]: Total sent " << totalBytes << " to " << this->_fd << std::endl;

}

// ============================ RESPONSE PART - [END] =================================

// getter
int HttpHandler::getfd(void) {
    return (this->_fd);
}

Server& HttpHandler::get_server(void) {
    return (this->_server);
}

// set server
void HttpHandler::set_server(Server &sv) {
    this->_server = sv;
}

void HttpHandler::set_server_list(std::vector<Server> &list) {
    this->_serverList = list;
}