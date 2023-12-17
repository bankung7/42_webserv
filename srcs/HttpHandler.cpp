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

    // TODO: function to get this request information
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
            
            // set the server // better to use one in response, just set the index
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
    this->set_location_attr(this->_uri);
    
    // >>>>>>>>>>>>>.. set file name <<<<<<<<<<<<<<<, //

    // set filename
    std::string name(this->_uri);
    std::string root(this->get_root());
    // std::cout << "location " << this->_location_path << std::endl;
    // std::cout << "location value " << this->_location_value << std::endl;
    // std::cout << "root " << root << std::endl;
    name.erase(0, this->_location_path.size() - 1);
    name.insert(name.begin(), root.begin(), root.end());

    // just set index.html, ignore index value for now.
    if (this->_uri.compare("/") == 0)
        name.append("html/index.html");

    name.erase(0, 1);

    std::cout << "[DEBUG]: file name " << name << std::endl;

    // get file name to send
    std::ifstream file;
    if (this->_location_path.compare("/images/") == 0)
        file.open(name.c_str(), std::ios::binary);
    else
        file.open(name.c_str(), std::ios::in);

    int fileSize = 0;
    std::string fileData;

    if (!file) {
        std::cout << "[ERROR]: the file cannot be open or no file exist" << std::endl;
    } else {
        fileData = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        fileSize = fileData.size();
    }

    // ================  create and send response header  ====================

    std::string content_type;
    if (this->_location_path.compare("/images/") == 0)
        content_type = std::string("image/jpeg");
    else
        content_type = std::string("text/html");

    std::stringstream ss;
    ss << "HTTP/1.1 200 OK\r\n"
        << "Content-type: " << content_type << "\r\n"
        << "Content-Length: " << fileSize << "\r\n\r\n"
        << fileData;

    // std::cout << fileSize << std::endl;
    // std::cout << ss.str().size() << std::endl;
    // std::cout << ss.str().size() - fileSize << std::endl;

    // send the header
    // std::string rHeader = std::string(ss.str());
    // std::vector<char> header;
    // header.insert(header.begin(), rHeader.begin(), rHeader.end());
    // send(this->_fd, header.data(), header.size(), MSG_NOSIGNAL);

    // ==================  create and send response body  =====================

    std::string res = ss.str();
    std::vector<char> msg;
    msg.insert(msg.begin(), res.begin(), res.end());

    // send in one shot
    int totalBytes = msg.size();
    int bytes = send(this->_fd, msg.data(), totalBytes, MSG_NOSIGNAL);

    if (bytes == -1)
        std::cout << "[ERROR]: send fail" << std::endl;

    // split send fail wit no idea.
    // flag MSG_NOSIGNAL
    // while (bytes < totalBytes) {
    //     int sentBytes;
    //     if (totalBytes - bytes < BUFFER_SIZE)
    //         sentBytes = send(this->_fd, msg.data() + bytes, BUFFER_SIZE, 0);
    //     else
    //         sentBytes = send(this->_fd, msg.data() + bytes, BUFFER_SIZE, 0);

    //     // TODO: error handling
    //     if (sentBytes <= 0) {
    //         std::cout << "[ERROR]: sending failed " << this->_fd << std::endl;
    //         break ;
    //     }

    //     bytes += sentBytes;
    // }

    // ==========================================================================

    std::cout << "[DEUBG]: Total sent " << bytes << " " << totalBytes << " to " << this->_fd << std::endl;

}

void HttpHandler::set_file(std::string input) {
    this->_file = std::string(input);
}

void HttpHandler::set_location_attr(std::string loc) {

    // std::cout << "[" << loc << "]" << std::endl;

    std::map<std::string, std::string> location = this->_server.get_location_block();

    // try the exact match, if found return
    if (location.find(loc) != location.end()) {
        // std::cout << "found exact match " << loc << std::endl;
        this->_location_path = std::string(loc);
        this->_location_value = std::string(location[loc]);
        return ;
    }

    // try most match by block to block
    std::map<std::string, std::string>::iterator it = location.begin();
    std::map<std::string, std::string>::iterator output;
    int match_value = 0;
    for (; it != location.end(); it++) {

        // std::cout << "matching " << it->first << " with " << loc << std::endl;

        // set if / and mot match any yet, by default
        if (match_value == 0 && it->first.compare("/") == 0)
            output = it;

        int match = -1;
        for (int i = 0; i < (int)it->first.size(); i++, match++) {
            if (loc[i] != it->first[i]) {
                // std::cout << i << " " << match << " " << loc[match] << " => " << it->first[match] << std::endl;
                break ;
            }
        }

        // std::cout << match << " with " << it->first[match] << " for " << it->first << std::endl;

        if (match > 0 && it->first[match] == '/' && match > match_value) {
            match_value = match;
            output = it;
        }

    }

    // std::cout << "[DEBUG]: found most match [" << match_value << "] long is [" << output->first << "]" << std::endl;

    // return
    if (match_value >= 0) {
        this->_location_path = std::string(output->first);
        this->_location_value = std::string(output->second);
        return ;
    }

    // return default path /, just back up check
    this->_location_path = std::string("/");
    this->_location_value = std::string(location["/"]);
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