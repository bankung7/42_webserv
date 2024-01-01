#include "HttpHandler.hpp"

HttpHandler::HttpHandler(void) {

}

HttpHandler::HttpHandler(int fd): _fd(fd), _status(READING), _serverIndex(-1) {

    this->_parameter["Host"] = std::string("");
    this->_parameter["Content-Length"] = std::string("0");
    this->_parameter["Connection"] = std::string("");
    this->_parameter["boundary"] = std::string("");
    this->_parameter["Content-Type"] = std::string("");

    this->_reqContentLength = 0;

    this->_body = std::string("");
    this->_bodyLength = 0;
    this->_isContinueRead = 0;
    this->_postType = 0;
    this->_maxClientBodySize = std::numeric_limits<std::size_t>::max(); // if not specified

    this->_tryFileStatus = 0;
    this->_isDirectory = 0;
    this->_isRedirection = 0;
    this->_isAutoIndex = 0;
    this->_isIndex = 0;
    this->_isCGI = 0;
    this->_fileSize = 0;
    this->_resStatusCode = 0;
    this->_resStatusText = std::string("");
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

void HttpHandler::set_res_content_type() {
    std::size_t startIndex, len;
    std::string type;

    startIndex = this->_filepath.rfind(".");
    if (startIndex != std::string::npos) {
        len = this->_filepath.size() - startIndex;
        type = std::string(this->_filepath.substr(startIndex, len));

        if (type.compare(".jpeg") == 0 || type.compare(".png") == 0 || type.compare(".jpg") == 0)
            this->_resContentType = std::string("image/*");

        return ;
    }
    this->_resContentType = std::string("text/html");
}

// getter
int HttpHandler::get_fd(void) const {
    return (this->_fd);
}

int HttpHandler::get_status(void) const {
    return (this->_status);
}

std::string HttpHandler::get_connection_type(void) {
    return (this->_parameter["Connection"]);
}

int HttpHandler::get_continue_read(void) const {
    return (this->_isContinueRead);
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
        // if body is reached, put to the body
        if (this->_isContinueRead == READING)
            this->_body.append(bf.data(), bytesRead);
        else
            this->_req.append(bf.data(), bytesRead);
    }

    // TODO : POST and DELETE method
    // std::cout << this->_req << std::endl;

    this->parsing_request();
}

void HttpHandler::parsing_request(void) {

    // protect case
    if (this->_status == CLOSED)
        return ;

    if (this->_isContinueRead == NOTSTART) {

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

            // check if the body exist
            if (line.compare("\r") == 0 && this->_parameter["Content-Length"].compare("0") != 0) {

                // move this to body
                std::getline(ss, line, '\n');
                // window case, it seem like it not send some part of the body in header when splited
                // macos, some part of the request is push into the first time read
                if (line.size() != 0) {
                    this->_body.append(this->_req.substr(this->_req.find(line)));
                }

                // =============
                if (this->_body.size() < this->_reqContentLength) {
                    this->_isContinueRead = READING;
                    return ;
                }

                break ;
            }

            // This is for header
            std::string attr;
            std::getline(lss, attr, ':');
            if (this->_parameter.find(attr) != this->_parameter.end()) {
                // std::cout << attr << std::endl;
                std::string value;
                std::getline(lss, value);
                this->remove_white_space(value);
                this->_parameter[attr] = std::string(value);

                // set content-length
                if (attr.compare("Content-Length") == 0 && this->_reqContentLength == 0) {
                    this->_reqContentLength = string_to_int(value);
                    std::cout << "Content-Length: " << this->_reqContentLength << std::endl;
                }

            }
        }
    }

    // if reading the body
    if (this->_isContinueRead == READING && (this->_body.size() < this->_reqContentLength)) {
        // std::cout << "current size: " << this->_body.size() << "/" << this->_reqContentLength << std::endl;
        return ;
    }

    // read completed
    if (this->_body.size() == this->_reqContentLength) {
        // std::cout << "=== received all body message : " << this->_body.size() << std::endl;

        this->_isContinueRead = COMPLETED;
        // split and set hostname and port
        std::string sport(this->_parameter["Host"]);
        sport.erase(0, sport.find(":") + 1);
        this->_parameter["Host"].erase(this->_parameter["Host"].find(sport) - 1, sport.size() + 1);
        std::stringstream pss(sport);
        pss >> this->_port;

        std::cout << "Content-Type: " << this->_parameter["Content-Type"] << std::endl;

        // check content type and set
        if (this->_parameter["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos) {
            this->_postType = URLENCODED;
            return ;
        }

        if (this->_parameter["Content-Type"].find("multipart/form-data") != std::string::npos) {
            this->_postType = FORMDATA;
            // set boundary
            std::size_t index = (this->_parameter["Content-Type"]).find("boundary=");
            this->_parameter["boundary"] = std::string((this->_parameter["Content-Type"]).substr(index + 9));
            remove_white_space(this->_parameter["boundary"]);
            this->_parameter["boundary"].insert(0, "--"); // inset the -- in front of boundary
            // std::cout << "set boudnary " << this->_parameter["boundary"] << std::endl;

            return ;
        }

        // std::cout << this->_body << std::endl;
        // std::cout << this->_req << std::endl;
        return ;
    }

    // protect overreading case
    if (this->_body.size() > this->_reqContentLength) {
        std::cout << "[WARNING]: Over reading" << std::endl;
        this->_isContinueRead = COMPLETED;
        set_res_status(404, "OVER READING");
    }

}

void HttpHandler::handle_response(void) {

    // protect case
    if (this->_status >= COMPLETED)
        return ;

    // Assign server block
    assign_server_block();
    // std::cout << "Server index: " << this->_serverIndex << std::endl;
    // std::cout << "Server host: " << this->_server[this->_serverIndex].get_server_name(this->_parameter["Host"]) << std::endl;
    // std::cout << "Server port: " << this->_server[this->_serverIndex].get_port() << std::endl;

    // match location
    assign_location_block();
    std::cout << "[DEBUG]: Location block was assigned to " << this->_loc << std::endl;

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

    // get the location block
    this->_loc.append(this->_server[this->_serverIndex].best_match_location(this->_url));
    this->_location.append(this->_server[this->_serverIndex].get_location(this->_loc));

    // std::cout << this->_loc << std::endl;
    // std::cout << this->_location << std::endl;

}

void HttpHandler::create_response(void) {

    std::size_t startIndex, length;

    startIndex = 0;
    // Set errorcode into the map
    while (1) {
        startIndex = this->_location.find("error_page:");
        if (startIndex == std::string::npos)
            break ;

        length = this->_location.find(";", startIndex + 1) - startIndex;
        parsing_error_code(std::string(this->_location.substr(startIndex + 11, length - 11)));
        this->_location.insert(startIndex + 10, "R");
    }

    // set root == MANDATORY
    startIndex = this->_location.find("root:");
    if (startIndex == std::string::npos) { // ================ this will fall down from the server itself
        this->_root = std::string(this->_server[this->_serverIndex].get_root()); // set root
    } else {
        length = this->_location.find(";", startIndex + 1) - startIndex;
        this->_root = std::string(this->_location.substr(startIndex + 5, length - 5)); // set root
    }

    this->_root.erase(0, 1); // remnove the first "/"
    this->_filepath.append(this->_root);

    std::string attribute;

    // check method request == MANDATORY
    startIndex = this->_location.find("allowedMethod:");
    length = this->_location.find(";", startIndex + 1) - startIndex + 1;
    attribute = std::string(this->_location.substr(startIndex, length));

    if (attribute.find(this->_method.c_str(), startIndex) == std::string::npos) {
        // std::cout << "Request Method " << this->_method << std::endl;
        // std::cout << "Allowed Method " << attribute << std::endl;
        std::cout << "\033[1;31mRequest " << this->_method << " method is not allowed\033[0m" << std::endl;
        set_res_status(405, "METHOD NOT ALLOWED");
        return ;
    }

    // check client limit
    startIndex = this->_location.find("client_max_body_size:");
    length = this->_location.find(";", startIndex + 1) - startIndex - 21;

    // std::cout << this->_location.substr(startIndex + 21, length) << std::endl;

    if (startIndex != std::string::npos) {
        this->_maxClientBodySize = string_to_size(this->_location.substr(startIndex + 21, length));
        // std::cout << "max size: " << this->_maxClientBodySize << std::endl;
        // std::cout << "body legnth: " << this->_reqContentLength << std::endl;
        if (this->_maxClientBodySize < this->_reqContentLength) {
            std::cout << "\033[1;33m[WARNING]: File is too large than server limit " << this->_maxClientBodySize << "\033[0m" << std::endl;
            set_res_status(413, "Content Too Large");
            // this->_tryFileStatus = -1;
            return ;
        }
    }

    // put the filename in the path
    // so the filepath should be root + url
    this->_filepath.append(this->_url);

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

    // file upload
    if (this->_location.find("allowedFileUpload:") != std::string::npos) {
        if (this->_location.find("allowedFileUpload:yes;") == std::string::npos) {
            std::cout << "\033[1;31Uploadfile is not allowed : " << std::string(this->_location.substr(startIndex + 7, length)) << "\033[0m" << std::endl;
            set_res_status(403, "Forbidden");
            return ;
        }
        uploading_task();
        // this->_tryFileStatus = -1;
        return ;
    }

    // for CGI == OPTIONAL =======================>

    // check if directory or not
    stat(this->_filepath.c_str(), &this->_fileInfo);
    // perror("stat");

    switch (this->_fileInfo.st_mode & S_IFMT) {
        case S_IFDIR: // if it is the directory
            this->_isDirectory = 1;
            break;
        default: // if nothing else
            // std::cout << "not support or file not found" << std::endl;
            break;
    }

    // other post method begin here =========================

    // is directory == OPTIONAL
    if (this->_isDirectory == 1) {

        std::cout << "\033[0;31mDIRECTORY\033[0;0m" << std::endl;

        // in case match location is directory but not / at the end
        if (this->_filepath[this->_filepath.size() - 1] != '/')
            this->_filepath.append("/");

        // index page accepted == OPTIONAL
        if (this->_location.find("index:") != std::string::npos) {
            // std::cout << indexPage << " is provided for this request" << std::endl;
            this->_isIndex = 1;
            startIndex = this->_location.find("index:");
            length = this->_location.find(";", startIndex + 1) - startIndex;
            this->_filepath.append(this->_location.substr(startIndex + 6, length - 6));
            return ;
        }

        // is autoindex == OPTIONAL, if off, don't set this in the structure
        // this might use cgi to generate the file into prepared folder
        if (this->_location.find("autoIndex:on;") != std::string::npos) {
            // std::cout << "Index Page requested" << std::endl;
            this->_isAutoIndex = 1;
            this->_filepath.append("indexofpage.html");
            return ;
        }

        // in case of no index and autoindex, set index.html as default
        this->_filepath.append("index.html");
    }

}

void HttpHandler::uploading_task(void) {

    //////////////////////// ============================= here

    // check if formdata process
    if (this->_postType == FORMDATA) {

        // read each boundary
        // std::getline() does not work as expected, fall to manual read by index
        // expected that payload was completed

        // std::cout << "============== start of body ==============" << std::endl;
        // std::cout << this->_body << std::endl;
        // std::cout << "============== end of body ==============" << std::endl;

        std::size_t sindex = 0, eindex = 0, length = 0;
        std::string boundary(this->_parameter["boundary"]);

        while (1) {

            // grab each block first
            sindex = this->_body.find(boundary, sindex);
            if (sindex == std::string::npos)
                break ;
            
            eindex = this->_body.find(boundary, sindex + boundary.size()); // eindex cant change as it would set the next position
            if (eindex == std::string::npos)
                break ;

            length = eindex - sindex - boundary.size();

            // std::cout << "end of start line: " << (int)this->_body[sindex + boundary.size() + 1] << std::endl;
            // std::cout << "end of end line: " << (int)this->_body[eindex - 2] << std::endl;

            std::string block(this->_body.substr(sindex + 2 + boundary.size(), length - 4)); // minus \r\n at the end of first line
            // std::cout << "============== start of block ==============" << std::endl;
            // std::cout << block << std::endl;
            // std::cout << "============== end of block ==============" << std::endl;

            // after we get the block, check the content-disposition and find the filename

            sindex = block.find("Content-Disposition:");
            
            if (sindex == std::string::npos) // =================== in case no disposition attiribute
                continue ;

            length = block.find("\r\n", sindex + 1) - sindex;
            std::string disposition(block.substr(sindex, length));
            // std::cout << disposition << std::endl;

            sindex = disposition.find("filename=\"");
            if (sindex == std::string::npos) // =================== in case no filename attribute
                continue ;

            length = disposition.find("\"", sindex + 10) - sindex - 10; // 1 from the from and 1 from the back
            std::string filename(disposition, sindex + 10, length);
            // std::cout << "filename: " << filename << std::endl;

            // get the file data
            sindex = block.find("\r\n\r\n");
            if (sindex == std::string::npos) // ==================== in case of not found \r\n\r\n // imposible case
                continue ;

            // std::cout << block.substr(sindex + 4) << std::endl;

            // open the file and write to it
            filename.insert(0, "/");
            std::size_t uindex = this->_location.find("uploadPath:");
            if (uindex == std::string::npos) // ============= if no uploadPathh found, use fall back to root
                filename.insert(0, this->_root);
            else { // ===== found
                length = this->_location.find(";", uindex + 1) - uindex - 11;
                filename.insert(0, this->_location.substr(uindex + 11, length));
                filename.erase(0, 1); // remove first /
            }
            std::cout << "filename: " << filename << std::endl;


            std::ofstream file(filename.c_str(), std::ios::binary);

            if (!file) { //================ cannot open the file
                // std::cout << filename << std::endl;
                std::cout << "[ERROR]: " << strerror(errno) << std::endl;
                set_res_status(403, "FOBIDDEN");
                sindex = eindex - 1; // return back 1 char
                continue ;
            }

            file << block.substr(sindex + 4);
            file.close();
            std::cout << "[DEBUG]: File save successfully" << std::endl;
            set_res_status(200, "OK file saved");
            this->_tryFileStatus = 2;

            sindex = eindex - 1; // return back 1 char
            // to next block
        }

        return ;
    }

    set_res_status(200, "OK");
    // return to the page before submit

}

void HttpHandler::set_res_status(int code, std::string text) {
    this->_resStatusCode = code;
    this->_resStatusText = std::string(text);

    if (code == 200)
        return ;

    // TODO: check if default error assigned, later
    std::map<int, std::string>::iterator it = this->_errorCode.find(code);
    if (it == this->_errorCode.end())
        this->_tryFileStatus = -1; // this will return system error page
    else {

        // guaruntee that custom error_page is exist before try_file

        std::cout << "the default error page was setup" << std::endl;
        this->_filepath.clear();
        this->_filepath.append(this->_root);
        this->_filepath.append(this->_errorCode[code]);

        // check if the custom error page is exist
        stat(this->_filepath.c_str(), &this->_fileInfo);
        if ((this->_fileInfo.st_mode & S_IFMT) == S_IFREG)
            return ;
        this->_tryFileStatus = -1;
        // std::cout << "error filepath: " << this->_filepath << std::endl;
    }

}

void HttpHandler::try_file(void) {

    // this step is reached when the filepath isset, even in the error code
    std::cout << "[DEBUG]: try file: " << this->_filepath << std::endl;
    std::cout << "try file status " << this->_tryFileStatus << std::endl;

    if (this->_isRedirection == 1 || this->_tryFileStatus == -1)
        return ;


    stat(this->_filepath.c_str(), &this->_fileInfo);

    // if file exist
    if ((this->_fileInfo.st_mode & S_IFMT) == S_IFREG) {

        // TODO: DELETE METHOD =========================
        if (this->_method.compare("DELETE") == 0) {
            
            // std::cout << "delete method" << std::endl;
            // std::cout << this->_req << std::endl;

            int status = std::remove(this->_filepath.c_str());
            if (status != 0) { // ==================== thing wend wrong for macos, it ask for permission to deleted
                std::cout << "[ERROR]: Permission denied" << std::endl;
                set_res_status(403, "NO PERMISSION");
                return ;
            }
            set_res_status(200, "OK");
            this->_tryFileStatus = -1; // if code can be default
            return ;
        }

        this->_file.open(this->_filepath.c_str(), std::ios::in);
        this->_fileSize = this->_fileInfo.st_size;
        set_res_content_type();
        if (this->_resStatusCode == 0)
            set_res_status(200, "OK");
        return ;
    }

    if (this->_resStatusCode == 0) {
        set_res_status(404, "NOT FOUND");
        try_file();
        return ;
    }

    // std::cout << "page not found " << this->_resStatusCode << std::endl;

}

void HttpHandler::content_builder(void) {

    std::string fileData("");

    if (this->_tryFileStatus != -1) {
        fileData = std::string(std::istreambuf_iterator<char>(this->_file), std::istreambuf_iterator<char>());
    } else if (this->_isRedirection != 1) {
        fileData.append(int_to_string(this->_resStatusCode));
        fileData.append(" : ");
        fileData.append(this->_resStatusText);
    }

    this->_file.close(); // close the file
    if (this->_fileSize == 0)
        this->_fileSize = fileData.size();

    // std::cout << "file size: " << this->_fileSize << std::endl;

    // create response header
    std::stringstream ss;
    ss << "HTTP/1.1 " << this->_resStatusCode << " " << this->_resStatusText << "\r\n";

    if (this->_isRedirection == 1) {
        ss << "Cache-Control: no-store\r\n";
        ss << "Location: " << this->_filepath << "\r\n\r\n";
    } else {
        ss << "Cache-Control: no-store\r\n";
        // ss << "Content-type: " << this->_resContentType << "\r\n"
        ss << "Content-type: " << "text/html;\r\n"
        << "Content-Length: " << this->_fileSize << "\r\n\r\n"
        << fileData;
    }


    std::string res(ss.str());
    std::vector<char> msg;
    msg.insert(msg.begin(), res.begin(), res.end());

    // std::cout << msg.data() << std::endl;

    int totalByte = msg.size();
    int sentByte = send(this->_fd, msg.data(), totalByte, 0);

    if (sentByte == -1)
        std::cout << "\033[;31m" << "[ERROR]: Something wrong while sending" << "\033[0m" << std::endl;
    else if (totalByte != sentByte)
        std::cout << "\033[;31m" << "[ERROR]: Sendin not whole file" << "\033[0m" << std::endl;
    else
        std::cout << "\033[;32m" << "[DEBUG]: Send completed " << totalByte << "\033[0m" << std::endl;
}


// errorpage set
void HttpHandler::error_page_set(int error, std::string text) {
    set_res_status(error, text);
    this->_filepath.clear();
    this->_filepath.append(this->_root);
    this->_filepath.append("/error/");
    this->_filepath.append(int_to_string(error));
    this->_filepath.append(".html");
    this->_filepath.erase(0, 1);
    std::cout << "error path " << this->_filepath << std::endl;
}

void HttpHandler::parsing_error_code(std::string str) {

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
    while (std::getline(ss, attr, ' ')) {
        int code = string_to_int(attr);
        this->_errorCode[code] = std::string(name);
    }
}

// Utils
void HttpHandler::remove_white_space(std::string& input) {
    int len = input.size();
    for (int i = 0; i < (int)len; i++) {
        if (input[i] == ' ') {
            input.erase(i, 1);
            i--;
            len = input.size();
        }
        else
            break;
    }

    // this line was wrong for a while, must check if any output is weird
    for (int i = input.size() - 1; i >= 0; i--) {
        if (input[i] == ' ' || input[i] == '\r' || input[i] == '\n') {
            input.erase(i, 1);
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

std::size_t HttpHandler::string_to_size(std::string str) {
    std::stringstream ss(str);
    std::size_t output;
    ss >> output;
    return (output);
}

std::string HttpHandler::int_to_string(int n) {
    std::stringstream ss;
    ss << n;
    std::string output(ss.str());
    return (std::string(output));
}
