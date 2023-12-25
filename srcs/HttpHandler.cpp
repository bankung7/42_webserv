#include "HttpHandler.hpp"

HttpHandler::HttpHandler(void) {

}

HttpHandler::HttpHandler(int fd): _fd(fd), _status(READING), _serverIndex(-1) {

    this->_parameter["Host"] = std::string("");
    this->_parameter["Connection"] = std::string("");

    this->_tryFileStatus = 0;
    this->_isDirectory = 0;
    this->_isRedirection = 0;
    this->_isAutoIndex = 0;
    this->_isIndex = 0;
    this->_isCGI = 0;
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

        // check case, defected
        if (bytesRead == 0) {
            // std::cout << "end of file case" << std::endl;
            this->set_status(CLOSED);
            break ;
        }

        // in case that nothing to read and error cannot separate
        if (bytesRead < 0) {
            // std::cout << "[ERROR]: The connection was closed or error occured" << std::endl;
            this->set_status(WRITING);
            break;
        }
        this->_req.append(bf.data(), bytesRead);
    }

    // std::cout << this->_req << std::endl;

    this->parsing_request();
}

void HttpHandler::parsing_request(void) {

    // protect case
    if (this->_status == CLOSED)
        return ;

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

    // split and set hostname and port
    std::string sport(this->_parameter["Host"]);
    sport.erase(0, sport.find(":") + 1);

    this->_parameter["Host"].erase(this->_parameter["Host"].find(sport) - 1, sport.size() + 1);

    std::stringstream pss(sport);
    pss >> this->_port;

    // std::cout << "Method: " << this->_method << std::endl;
    // std::cout << "URL: " << this->_url << std::endl;
    // std::cout << "Version: " << this->_version << std::endl;
    // std::cout << "Host: " << this->_parameter["Host"] << std::endl;
    // std::cout << "Connection: " << this->_parameter["Connection"] << std::endl;

}

void HttpHandler::handle_response(void) {

    // protect case
    if (this->_status == CLOSED)
        return ;

    // Assign server block
    assign_server_block();
    // std::cout << "Server index: " << this->_serverIndex << std::endl;
    // std::cout << "Server host: " << this->_server[this->_serverIndex].get_server_name(this->_parameter["Host"]) << std::endl;
    // std::cout << "Server port: " << this->_server[this->_serverIndex].get_port() << std::endl;

    // match location
    assign_location_block();
    std::cout << "[DEBUG]: Location block was assigned to " << this->_path << std::endl;

    // create response
    create_response();

    // try file
    try_file();

    // content builder
    content_builder();
}

void HttpHandler::assign_server_block(void) {

    // loop check with port and assigned the first found for default,
        // or if it exact match just return
        // continue loop if found

    for (int i = 0; i < (int)this->_server.size(); i ++) {

        // if not the same port, check by hostfd that first accept
        if (this->_server[i].get_port() != this->_port) {
            continue ;
        }

        // check if the server index was not assign to be default
        if (this->_serverIndex == -1)
            this->_serverIndex = i;

        // check if the server name exact match
        if (this->_server[i].has_server_name(this->_parameter["Host"]) == 1) {
            this->_serverIndex = i;
            return ;
        }
    }
}

void HttpHandler::assign_location_block(void) {

    // std::cout << "checking url => " << this->_url << std::endl;

    // check if there is only /, from the backward
    if (this->_url.compare("/") == 0) {

        this->_isDirectory = 1;

        this->_path.append("/");
        this->_location.append(this->_server[_serverIndex].get_location("/"));

        this->_filename.append(this->_url);

        // std::cout << "path: " << this->_path << std::endl;
        // std::cout << "location: " << this->_location << std::endl;
        // std::cout << "filename: " << this->_filename << std::endl;
        return ;
    }

    // example just snip / -> /
    std::string path(this->_url);
    if (path[path.size() - 1] != '/') {
        // std::cout << "not a directory case" << std::endl;
        path.erase(path.rfind("/"));
    }
    else
        this->_isDirectory = 1;

    // TODO: in case of double directory //
    this->_path.append(this->_server[this->_serverIndex].best_match_location(path));
    this->_filename.append(this->_url);

    // std::cout << "paht to check [" << this->_path << "] " << path << std::endl;

    if (this->_path.size() == 1)
        this->_filename.erase(0, this->_path.size() - 1);
    else
        this->_filename.erase(0, this->_path.size());

    this->_location.append(this->_server[this->_serverIndex].get_location(this->_path));

    // std::cout << "path: " << this->_path << std::endl;
    // std::cout << "location: " << this->_location << std::endl;
    // std::cout << "filename: " << this->_filename << std::endl;

}

void HttpHandler::create_response(void) {

    int startIndex, length;

    // set root == MANDATORY
    startIndex = this->_location.find("root:");
    length = this->_location.find(";", startIndex + 1) - startIndex;
    this->_lroot = std::string(this->_location.substr(startIndex + 5, length - 5)); // set lroot for error page
    this->_filepath.append(this->_location.substr(startIndex + 5, length - 5));

    std::string attribute;

    // check method request == MANDATORY
    startIndex = this->_location.find("allowedMethod:");
    length = this->_location.find(";", startIndex + 1) - startIndex + 1;
    attribute = std::string( this->_location.substr(startIndex, length));

    // std::cout << "Request Method " << this->_method << std::endl;
    // std::cout << "Allowed Method " << attribute << std::endl;

    if (attribute.find(this->_method.c_str(), startIndex) == std::string::npos) {
        std::cout << "\033[1;31mRequest " << this->_method << " method is not allowed\033[0m" << std::endl;
        // this->_filepath.append("/error/405.html");
        set_res_status(405, "METHOD NOT ALLOWED");
        return ;
    }

    // Redirection == OPTIONAL
    if (this->_location.find("return:") != std::string::npos) {
        
        this->_isRedirection = 1;
        this->_filepath.clear();

        startIndex = this->_location.find("return:");
        length = this->_location.find(";", startIndex + 1) - startIndex - 7;
        
        std::cout << "\033[1;31mRedirection case : " << std::string(this->_location.substr(startIndex + 7, length)) << "\033[0m" << std::endl;

        this->_filepath = std::string(this->_location.substr(startIndex + 7, length));
        set_res_status(301, "MOVED PERMANENTLY");
        return;
    }

    // for CGI == OPTIONAL =======================>

    // put the filename in the path
    if (this->_filename.size() > 0)
        this->_filepath.append(this->_filename);

    set_res_status(200, "OK");

    // std::cout << "current filename : " << this->_filepath << std::endl;

    // is directory == OPTIONAL
    if (this->_isDirectory == 1) {

        std::cout << "\033[0;31mDIRECTORY\033[0;0m" << std::endl;

        // is autoindex == OPTIONAL
        if (this->_location.find("autoIndex:") != std::string::npos) {
            // std::cout << "Index Page requested" << std::endl;
            this->_isAutoIndex = 1;
            this->_filepath.append("/indexofpage.html");
            return ;
        }

        // index page accepted == OPTIONAL
        if (this->_location.find("index:") != std::string::npos && this->_filename.size() == 1) {
            // std::cout << indexPage << " is provided for this request" << std::endl;
            this->_isIndex = 1;
            startIndex = this->_location.find("index:");
            length = this->_location.find(";", startIndex + 1) - startIndex;
            this->_filepath.append(this->_location.substr(startIndex + 6, length - 6));
            return ;
        }
        
        // in case of directory but path not authorized to access
        set_res_status(404, "NOT FOUND");
    }

}

void HttpHandler::try_file(void) {

    if (this->_tryFileStatus == -1 || this->_isRedirection == 1)
        return ;

    // remove the first /
    this->_filepath.erase(0, 1);
    std::cout << "try file: " << this->_filepath << std::endl;

    this->_file.open(this->_filepath.c_str(), std::ios::in);

    // if file canot be open for some reason, assume this as no file found
    // so set it to 404, check for default later
    if (!this->_file) {
        // watchout this case, it is potential to be never ending loop
        std::cout << "\033[;31m" << "[ERROR]: file => " << this->_filepath << " can't be open" << "\033[0m" << std::endl;
        set_res_status(404, "NOT FOUND");
        this->_file.open(this->_filepath.c_str(), std::ios::in);
    }

}

void HttpHandler::content_builder(void) {

    int fileSize = 0;
    std::string fileData("");

    if (this->_tryFileStatus != -1) {
        fileData = std::string(std::istreambuf_iterator<char>(this->_file), std::istreambuf_iterator<char>());
    } else if (this->_isRedirection != 1) {
        fileData.append(int_to_string(this->_resStatusCode));
        fileData.append(" : ");
        fileData.append(this->_resStatusText);
    }
    
    this->_file.close();
    fileSize = fileData.size();

    // create response header
    std::stringstream ss;
    ss << "HTTP/1.1 " << this->_resStatusCode << " " << this->_resStatusText << "\r\n";

    if (this->_isRedirection == 1) {
        ss << "Location: " << this->_filepath << "\r\n\r\n";
    } else {
        ss << "Content-type: text/html\r\n"
        << "Content-Length: " << fileSize << "\r\n\r\n"
        << fileData;
    }

    std::string res(ss.str());
    std::vector<char> msg;
    msg.insert(msg.begin(), res.begin(), res.end());

    int totalByte = msg.size();
    int sentByte = send(this->_fd, msg.data(), totalByte, 0);

    if (sentByte == -1)
        std::cout << "\033[;31m" << "[ERROR]: Something wrong while sending" << "\033[0m" << std::endl;
    else if (totalByte != sentByte)
        std::cout << "\033[;31m" << "[ERROR]: Sendin not whole file" << "\033[0m" << std::endl;
    else
        std::cout << "\033[;32m" << "[ERROR]: Send completed " << totalByte << "\033[0m" << std::endl;
}

void HttpHandler::set_res_status(int code, std::string text) {
    this->_resStatusCode = code;
    this->_resStatusText = std::string(text);

    if (code == 200)
        return ;
    
    // check if default error assigned, later

    this->_tryFileStatus = -1;

}

// errorpage set
void HttpHandler::error_page_set(int error, std::string text) {
    set_res_status(error, text);
    this->_filepath.clear();
    this->_filepath.append(this->_lroot);
    this->_filepath.append("/error/");
    this->_filepath.append(int_to_string(error));
    this->_filepath.append(".html");
    this->_filepath.erase(0, 1);
    std::cout << "error path " << this->_filepath << std::endl;
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

int HttpHandler::string_to_int(std::string str) {
    std::stringstream ss(str);
    int output;

    ss >> output;

    return (output);
}

std::string HttpHandler::int_to_string(int n) {
    std::stringstream ss;
    ss << n;
    std::string output(ss.str());
    return (std::string(output));
}
