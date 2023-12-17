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

    // std::cout << this->_req << std::endl;

    check_host();

    // get start line
    std::string line;
    std::getline(ss, line, '\n');

    std::stringstream sline(line);
    std::getline(sline, this->_method, ' '); // get method
    std::getline(sline, this->_uri, ' '); // get resource
    std::getline(sline, this->_version); // get version

    // std::cout << "[DEBUG]: Request Method => " << this->_method << std::endl;
    // std::cout << "[DEBUG]: Request Resource => " << this->_uri << std::endl;

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
            // std::cout << "[DEBUG]: " << this->_host << " with port " << strport << std::endl;

            // convert
            std::stringstream iss(strport);
            int port;
            iss >> port;

            // loop check the server_name in this port
            this->_server_index = -1;
            for (int i = 0; i < (int)this->_serverList.size(); i++) {
                if (this->_serverList[i].get_port() == port) {

                    // set to defautl server, if not found server name yet
                    if (this->_server_index == -1)
                        this->_server_index = i;

                    // check if it match the server_name
                    int index = this->_serverList[i].check_server_name(host);
                    if (index != -1) {
                        this->_server_index = i;
                        break ;
                    }
                }
            }
            
            // set the server
            this->set_server(this->_serverList[this->_server_index]);

            break ;
        }

    }

    return (0);
}

// ============================ REQUEST PART - [END] =================================
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// ============================ RESPONSE PART - [START] ==============================

// response handling
void HttpHandler::handlingResponse(void) {

    // TODO: create response
    Server sv = this->_server;

    // try matching the location block by _uri
    std::cout << "[DEBUG]: try find the location [" << this->_uri << "]" << std::endl;

    // check === no / === in config file.
    this->set_location_value(sv.get_location(this->_uri));
    // std::cout << this->_location_value << std::endl;
    
    // >>>>>>>>>>>>>.. set file name <<<<<<<<<<<<<<<, //

    // set filename
    std::string name(this->_uri);
    std::string root(this->get_root());
    std::cout << "root " << root << std::endl;
    std::cout << "file name " << name << std::endl;

    // get file name to send
    std::string fileName(sv.get_root());
    fileName.append(std::string("/html/index.html"));
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

void HttpHandler::set_file(std::string input) {
    this->_file = std::string(input);
}

void HttpHandler::set_location_value(std::string input) {
    this->_location_value = std::string(input);
}

std::string HttpHandler::get_root(void) {
    std::size_t start = this->_location_value.find("root:");
    // if not find, get root from server block instead
    if (start == std::string::npos)
        return (std::string(this->_server.get_root()));

    start += 5; // move to the start of path
    std::string root(this->_location_value, (int)start, this->_location_value.find(";", (int)start) - start);
    // std::cout << root << std::endl;

    this->_root = std::string(root);

    return (std::string(root));
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