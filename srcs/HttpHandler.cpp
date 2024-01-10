#include "HttpHandler.hpp"

HttpHandler::HttpHandler(void)
{
}

HttpHandler::HttpHandler(int fd, std::vector<Server> server) : _fd(fd), _status(READING_HEADER), _server(server), _serverIndex(-1)
{

    this->_parameter["Host"] = std::string("");
    this->_parameter["Content-Type"] = std::string("");
    this->_parameter["Content-Length"] = std::string("0");
    this->_parameter["boundary"] = std::string("");
    this->_parameter["Connection"] = std::string("keep-alive"); // default to keep-alive
    this->_parameter["Keep-Alive"] = std::string("");

    this->_reqContentLength = 0;

    std::time(&this->_timeout); // default
    this->_timeout += KEEP_ALIVE_TIME_OUT;

    this->_postType = 0;
    this->_maxClientBodySize = std::numeric_limits<std::size_t>::max(); // if not specified

    // cgi part
    this->_isCGI = 0;
    this->_cgiState = 0;
    this->_queryString = std::string("");

    this->_tryFileStatus = 0;

    this->_isDirectory = 0;
    this->_isRedirection = 0;
    this->_isAutoIndex = 0;
    this->_isIndex = 0;
    this->_fileSize = 0;
    this->_bytesSent = 0;
    this->_response.clear();

    this->_resStatusCode = 0;
    this->_resStatusText = std::string("");
}

HttpHandler::~HttpHandler(void)
{
}

// setter
void HttpHandler::set_status(int status)
{
    this->_status = status;
}

void HttpHandler::set_server(std::vector<Server> &server)
{
    this->_server = server;
}

void HttpHandler::set_res_content_type()
{

    std::cout << S_DEBUG << this->_filepath << std::endl;

    std::string type;
    std::size_t index = this->_filepath.rfind(".");

    if (index != std::string::npos)
    {
        type = this->_filepath.substr(index);
        // std::cout << "type : " << type << std::endl;

        if (type.compare(".jpeg") == 0 || type.compare(".png") == 0 || type.compare(".jpg") == 0)
        {
            this->_resContentType = std::string("image/*");
            return;
        }
        if (type.compare(".html") == 0)
        {
            this->_resContentType = std::string("text/html");
            return;
        }
    }
    this->_resContentType = std::string("text/plain");
}

// getter
int HttpHandler::get_fd(void) const
{
    return (this->_fd);
}

int HttpHandler::get_status(void) const
{
    return (this->_status);
}

std::string HttpHandler::get_connection_type(void)
{
    return (this->_parameter["Connection"]);
}

std::time_t HttpHandler::get_time_out(void) const
{
    return (this->_timeout);
}

// [OK] handle request
void HttpHandler::handle_request(void)
{
    if (this->_status <= READING_BODY) {

        reading_phase();

        // not finish reading
        if (this->_status <= READING_BODY)
            return ;
    }
    
    // finish reading, starting processing
    if (this->_status == PROCESSING) {
        processing();
    }

    // try file phase
    if (this->_status == TRY_FILE_PHASE) {
        // std::cout << "try file" << std::endl;
        try_file();
    }

    // content builing
    if (this->_status == CONTENT_PHASE) {
        // std::cout << "content builder" << std::endl;
        content_builder();
    }

}

// [OK] Reading Phase
void HttpHandler::reading_phase(void) {

    std::vector<char> bf(BUFFER_SIZE);
    int bytesRead;

    bf.clear();
    bytesRead = recv(this->get_fd(), bf.data(), BUFFER_SIZE, 0);

    std::cout << S_DEBUG << "Request Bytes read: " << bytesRead << S_END;

    // error case
    if (bytesRead < 0)
    {
        std::cout << S_ERROR << "Client [" << this->_fd << "] has something error while reading" << S_END;
        set_status(CLOSED); // it might be set to other error
    }
    // client closed the connection
    else if (bytesRead == 0)
    {
        std::cout << S_WARNING << "Client [" << this->_fd << "]closed the connection" << S_END;
        set_status(CLOSED);
    }
    // bytes read > 0
    else
    {
        // normal case, try to parsing the request and check that all message received

        if (this->_status <= READING_HEADER)
            this->_req.append(bf.data(), bytesRead); // write to (req) header
        else
            this->_body.append(bf.data(), bytesRead); // write to body

        // when reading to body
        if (this->_status == READING_BODY)
        {
            std::cout << "==== reading to body ====" << std::endl;
            // if completed, to epollout
            if (this->_body.size() == this->_reqContentLength)
            {
                std::cout << S_DEBUG << "Request body size: " << this->_reqContentLength << "/" << this->_body.size() << S_END;
                this->_status = PROCESSING;
                return;
            }

            // if not completed, queue in epollin
            return;
        }

        // check \r\n\r\n
        if (this->_status <= READING_HEADER && this->_req.find("\r\n\r\n") != std::string::npos)
        {
            std::cout << "==== end of header found, set up header ====" << std::endl;
            // split after \r\n\r\n to body if exist, and remove it for req
            this->_body.append(this->_req.substr(this->_req.find("\r\n\r\n") + 4));
            this->_req.erase(this->_req.find("\r\n\r\n"));

            this->_status = READING_BODY;

            // set up header parameter
            setup_header();

            std::cout << "[DEBUG]: Request body size: " << this->_reqContentLength << "/" << this->_body.size() << S_END;
           
            if (this->_body.size() >= this->_reqContentLength)
            {
                this->_status = PROCESSING;
                return;
            }

            return;
        }
    }
}

// [OK] setting up the header
void HttpHandler::setup_header(void)
{
    std::stringstream ss(this->_req);
    std::string line;

    // get first line // must 3 part
    // get method
    std::getline(ss, this->_method, ' ');
    remove_white_space(this->_method);

    // get resource
    std::getline(ss, this->_url, ' ');
    remove_white_space(this->_url);

    // get version
    std::getline(ss, this->_version);

    // loop put all the set parameter
    while (std::getline(ss, line, '\n'))
    {
        // std::cout << "line: " << line << std::endl;

        std::stringstream lss(line);

        // This is for header
        std::string attr;
        std::getline(lss, attr, ':');
        if (this->_parameter.find(attr) != this->_parameter.end())
        {
            // std::cout << "[DEBUG]: found " << attr << std::endl;

            std::string value;
            std::getline(lss, value);
            this->remove_white_space(value);
            this->_parameter[attr] = std::string(value);

            // std::cout << "set " << attr << " as " << value << std::endl;

            // for Host
            if (attr.compare("Host") == 0)
            {
                // if port does not specified
                std::size_t index = value.find(":");

                // set default 80 if it not specify
                if (index == std::string::npos)
                {
                    this->_parameter["Host"] = std::string(value);
                    this->_port = 80;
                }
                else
                {
                    this->_port = string_to_int(value.substr(value.find(":") + 1));
                    this->_parameter["Host"] = std::string(value);
                    this->_parameter["Host"].erase(this->_parameter["Host"].find(":"));
                }
                continue;
            }

            // for Connection
            if (attr.compare("Connection") == 0)
            {
                // Connection: keep-alive
                if (value.compare("keep-alive") == 0)
                {
                    std::time(&this->_timeout);

                    // No Keep-Alive in the request, set default
                    if (this->_parameter["Keep-Alive"].size() == 0)
                    {
                        this->_timeout += KEEP_ALIVE_TIME_OUT;
                    }
                    // Keep-alive is set in the request
                    else
                    {
                        std::stringstream kss(value);
                        std::string kvalue;
                        std::getline(kss, kvalue, ',');
                        remove_white_space(kvalue);
                        if (kvalue.size() > 0)
                        {
                            int ktime = string_to_int(kvalue.substr(kvalue.find("=") + 1));
                            this->_timeout += ktime;
                            std::cout << "ktime " << ktime << std::endl;
                        }
                        else
                        {
                            this->_timeout += KEEP_ALIVE_TIME_OUT;
                        }
                    }
                }
                // Connection: closed, and other if does not match keep-alive
                else
                {
                    std::time(&this->_timeout);
                    this->_timeout *= 1.5;
                }
                continue;
            }

            // for Content-length
            if (attr.compare("Content-Length") == 0 && this->_reqContentLength == 0)
            {
                this->_reqContentLength = string_to_size(value);
                std::cout << S_DEBUG << "Content-Length: " << this->_reqContentLength << S_END;
                continue;
            }

            // for Content-type
            if (attr.compare("Content-Type") == 0 && this->_parameter["Content-Type"].size() != 0) {
                
                // formdata
                if (this->_parameter["Content-Type"].find("multipart/form-data") != std::string::npos)
                {
                    this->_postType = FORMDATA;

                    // set boundary
                    std::size_t index = (this->_parameter["Content-Type"]).find("boundary=");
                    this->_parameter["boundary"] = std::string((this->_parameter["Content-Type"]).substr(index + 9));
                    remove_white_space(this->_parameter["boundary"]);
                    this->_parameter["boundary"].insert(0, "--"); // inset the -- in front of boundary
                    // std::cout << "set boudnary " << this->_parameter["boundary"] << std::endl;
                    continue;
                }

                // urlencoded
                if (this->_parameter["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos)
                {
                    this->_postType = URLENCODED;
                    continue;
                }

            }
        }
    }

    // if no content-type in the request, use defualt text/html
    if (this->_parameter["Content-Type"].size() == 0)
        this->_parameter["Content-Type"].append("text/plain"); /// try here

}

// [OK]
void HttpHandler::assign_server_block(void)
{
    // as the subject is not clear, we will design this step as to exactly match host:port.
    for (int i = 0; i < (int)this->_server.size(); i++)
    {

        // if port not match, skip
        if (this->_server[i].get_port() != this->_port)
        {
            continue;
        }

        // // check if the server index was not assign to be default
        // if (this->_serverIndex == -1)
        //     this->_serverIndex = i;

        // if the port match but no server_name defined
        if (this->_server[i].is_server_name_defined() == -1)
        {
            // std::cout << "no servername defined" << std::endl;
            this->_serverIndex = i;
            return;
        }

        // if the port and  server name match
        if (this->_server[i].has_server_name(this->_parameter["Host"]) == 1)
        {
            // std::cout << this->_parameter["Host"] << " " << this->_server[i].get_server_name(this->_parameter["Host"]) << std::endl;
            this->_serverIndex = i;
            return;
        }

    }

    set_res_status(404, "Not Found");
    this->_parameter["Connection"] = "closed";
    this->_status = CONTENT_PHASE; // as no servername match
}

// [OK]
void HttpHandler::assign_location_block(void)
{

    // get the location block
    this->_loc.append(this->_server[this->_serverIndex].best_match_location(this->_url));
    this->_location.append(this->_server[this->_serverIndex].get_location(this->_loc));

    // std::cout << this->_loc << std::endl;
    // std::cout << this->_location << std::endl;
}

// [ON PROCESS] proceesing the request
void HttpHandler::processing(void) {

    std::cout << S_INFO << "Processing state" << S_END;

    // assign server block and set location
    assign_server_block();
    assign_location_block();

    std::size_t startIndex, length;
    startIndex = 0;


    // ==> error_page [OPTIONAL]
    while (1) {
        startIndex = this->_location.find("error_page:");
        if (startIndex == std::string::npos)
            break ;

        // found the directive, in location block only
        length = this->_location.find(";", startIndex + 1) - startIndex;
        parsing_error_code(this->_location.substr(startIndex + 11, length - 11));
        this->_location.insert(startIndex + 10, "R"); // mark as found
    }

    // std::cout << S_INFO << "error_page: " << this->_errorCode.size() << S_END;
    // =========>


    // ==> set root [MANDATORY]
    startIndex = this->_location.find("root:");
    // root not exist in the location block, fall back to server block
    if (startIndex == std::string::npos) {
        this->_root = std::string(this->_server[this->_serverIndex].get_root());
    }
    // found root in the location block
    else {
        length = this->_location.find(";", startIndex + 1) - startIndex;
        this->_root = std::string(this->_location.substr(startIndex + 5, length - 5));
    }

    if (this->_root[0] == '/')
        this->_root.erase(0, 1); // remove the first /

    // std::cout << S_INFO << "root: " << this->_root << S_END;
    // =========>


    // create the resource path
    this->_filepath = std::string(this->_root);
    this->_filepath.append(this->_url);

    std::cout << S_INFO << "filepath => " << this->_filepath << S_END;

    // [TODO]: check if is file or directory exist
    if (stat(this->_filepath.c_str(), &this->_fileInfo) == 0) {

        // file check
        if (S_ISREG(this->_fileInfo.st_mode)) {
            std::cout << S_INFO << "this is a file, existing" << S_END;
        }
        // directory check
        else if (S_ISDIR(this->_fileInfo.st_mode)) {
            
            // the index directive found
            if (this->_location.find("index:") != std::string::npos) {
                startIndex = this->_location.find("index:");
                length = this->_location.find(";", startIndex + 1) - startIndex;
                this->_filepath.append(this->_location.substr(startIndex +  6, length - 6));
                std::cout << S_DEBUG << "default index page " << this->_location.substr(startIndex +  6, length - 6) << " has setted up" << S_END;
            }
            // if the autoindex directive is on [OPTIONAL]
            else if (this->_location.find("autoIndex:on;") != std::string::npos) {
                startIndex = this->_location.find("autoIndex:on;");
                std::cout << S_INFO << "generating index listing page" << S_END;
            } 
            // the default case if the directory is request but no 2 above
            else {
                this->_filepath.append("/index.html");
            }
        }
    } 
    // file or directory not found
    else {
        std::cout << this->_location.find("allowedFileUpload:yes;") << std::endl;
        // if this is not a redirection and uploadfile and cgi also
        if(this->_location.find("return: ") == std::string::npos &&
            this->_location.find("allowedFileUpload:yes;") == std::string::npos &&
            this->_loc.compare("/cgi-bin/") != 0) {
            std::cout << S_WARNING << "No file or directory exists" << S_END;
            set_res_status(404, "NOT FOUND");
            return ;
        }

        // std::cout << "potential redirection case" << std::endl;
    }
    // =========>


    // ==> allowedMethod [MANDATORY]
    startIndex = this->_location.find("allowedMethod:");
    length = this->_location.find(";", startIndex + 1) - startIndex;
    std::string locationMethod(this->_location.substr(startIndex + 14, length - 14));

    // if method is not found, mean not allowed
    if (locationMethod.find(this->_method) == std::string::npos) {
        std::cout << S_WARNING << this->_method << " is not ALLOWED" << S_END;
        set_res_status(405, "Method Not Allowed");
        return ;
    }

    // let check if the method is not GET,POST,DELETE, 501 return
    if (std::string(",GET,POST,DELETE,").find(this->_method) == std::string::npos) {
        std::cout << S_WARNING << this->_method << " is not implemented for this server" << S_END;
        set_res_status(501, "Not Implemented");
        return ;
    }
    // =========>


    // for DELETE method case
    if (this->_method.compare("DELETE") == 0) {

        // check the right to delete, support file only
        if (this->_isDirectory == 0 && (this->_filepath.c_str(), W_OK) == 0) {
            std::remove(this->_filepath.c_str());
            set_res_status(204, "No Content");
            return ;
        }
        // no right to delete, include if it is a folder, we not support folder deletion
        else {
            std::cout << S_WARNING << this->_method << " is not ALLOWED" << S_END;
            set_res_status(403, "FORBIDDEN");
            return ;
        }

    }   
    // =========>


    // ==> maxClientBodySize [OPTIONAL]
    startIndex = this->_location.find("client_max_body_size:");
    if (startIndex != std::string::npos) {
        length = this->_location.find(";", startIndex + 1) - startIndex;
        this->_maxClientBodySize = string_to_size(this->_location.substr(startIndex + 21, length - 21));

        // check if it goes over limit
        if (this->_maxClientBodySize < this->_reqContentLength) {
            std::cout << S_WARNING << "the body is over the value limited by server" << S_END;
            set_res_status(413, "CONTENT TOO LARGE");
            return ;
        }
    }
    // std::cout << S_INFO << "client max body size: " << this->_maxClientBodySize << S_END;
    // =========>


    // ==> Redirection [OPTIONAL]
    startIndex = this->_location.find("return: "); // must have space
    if (startIndex != std::string::npos) {
        length = this->_location.find(";", startIndex + 1) - startIndex;

        std::stringstream returnStream(this->_location.substr(startIndex + 8, length - 8));
        std::string returnAttr;
        
        // get code first
        std::getline(returnStream, returnAttr, ' ');
        remove_white_space(returnAttr);
        int code = string_to_int(returnAttr);

        // get path or text second
        std::getline(returnStream, returnAttr);
        remove_white_space(returnAttr);

        // split the case, only 3xx series will redirect to other url as specified
        if (code >= 300 && code <= 399) {
            std::cout << S_INFO << "redirection to site" << S_END;
            this->_isRedirection = 1;
            this->_filepath = std::string(returnAttr);
            set_res_status(code, "MOVED");
            return ;
        } 
        // for code 400 - 599, return as text
        else if (code >= 400 && code <= 599) {
            std::cout << S_INFO << "redirection with code" << S_END;
            returnAttr.erase(0, 1); // remove " begin
            returnAttr.erase(returnAttr.size() - 1); // remove " end
            set_res_status(code, returnAttr);
            return ;
        } 
        // other code that not in criteria, will use 404 for default [protected case]
        else {
            std::cout << S_INFO << "unknown res code" << S_END;
            set_res_status(404, "NOT FOUND");
            return ;
        }

    }
    // std::cout << S_INFO << "redirection not found" << S_END;
    // =========>

    
    // upload file request
    if (this->_location.find("allowedFileUpload:yes;") != std::string::npos) {
        std::cout << S_INFO << "upload file requested" << S_END;
        uploading_task();
        return ;
    }
    // =========>


    // [TODO]: CGI [OPTIONAL]
    if (this->_loc.compare("/cgi-bin/") == 0) {
        std::cout << S_DEBUG << "cgi request" << S_END;

        // support only get and post
        if (std::string(",GET,POST,").find(this->_method) == std::string::npos) {
            std::cout << S_INFO << "CGI not support this request" << S_END;
            set_res_status(405, "Method Not Allowed");
            return ;
        }

        // accept only urlencoded type
        std::cout << this->_parameter["Content-Type"] << std::endl;
        if (this->_parameter["Content-Type"].compare("text/plain") == 0) {
            // put the query string to body
            startIndex = this->_url.find("?");
            if (startIndex != std::string::npos) {
                this->_queryString.clear();
                this->_queryString.append(this->_url.substr(startIndex + 1)); // not include ?
                this->_url.erase(startIndex);
            }
            handle_cgi();
            return ;
        } else if (this->_parameter["Content-Type"].compare("application/x-www-form-urlencoded") == 0) {
            handle_cgi();
            return ;
        }
        // not support
        else {
            std::cout << S_INFO << "CGI not support this request" << S_END;
            set_res_status(501, "Not Implemented");
            return ;
        }

    }

    // =========>


    // if normal file (resource) request
    this->_status = TRY_FILE_PHASE;
}

// [ON PROCESS]
void HttpHandler::try_file(void)
{

    // this step is reached when the filepath isset, even in the error code
    // std::cout << "try file status " << this->_tryFileStatus << std::endl;

    std::cout << S_INFO << "try to open file: " << this->_filepath << std::endl;

    // if file request not found, when it set from set_res_status as error page
    if (access(this->_filepath.c_str(), F_OK) == -1)
    {
        // std::cout << YELLOW << "[WARNING]: File does not exist" << std::endl;
        set_res_status(404, "Not Found");
        this->_status = CONTENT_PHASE;
        return ;
    }

    // check the permission
    if (access(this->_filepath.c_str(), R_OK) == -1) {
        // dont have permission to read it
        set_res_status(403, "Forbidden");
        this->_status = CONTENT_PHASE;
        return ;
    }

    // get the file size
    stat(this->_filepath.c_str(), &this->_fileInfo);

    this->_file.open(this->_filepath.c_str(), std::ios::in);

    // check if the file cannot open for some reasone
    if (!this->_file.is_open()) {
        std::cout << S_WARNING << "The file cannot be open for some reason" << std::endl;
        set_res_status(500, "Internal Server Error");
        this->_status = CONTENT_PHASE;
        return ;
    }

    this->_fileSize = this->_fileInfo.st_size;
    set_res_content_type(); // try to set the content page from extension

    if (this->_resStatusCode == 0)
        set_res_status(200, "OK");

    this->_status = CONTENT_PHASE;
}

// [ON PROCESS]
void HttpHandler::set_res_status(int code, std::string text)
{
    this->_resStatusCode = code;
    this->_resStatusText = std::string(text);

    if (code < 400 || this->_isCGI == 1) {
        // std::cout << "OK or CGI completed" << std::endl;
        this->_status = CONTENT_PHASE;
        return;
    }

    // TODO: check if default error assigned, later
    std::map<int, std::string>::iterator it = this->_errorCode.find(code);

    this->_status = CONTENT_PHASE;

    // check in the location first
    if (it == this->_errorCode.end())
    {
        this->_tryFileStatus = -1;
        return ;
    }
    
    // guaruntee that custom error_page is exist before try_file
    // std::cout << "the default error page was setup" << std::endl;
    this->_filepath.clear();
    this->_filepath.append(this->_root);
    this->_filepath.append(this->_errorCode[code]);

    // check if the custom error page is exist
    if (access(this->_filepath.c_str(), F_OK | R_OK) != -1)
    { 
        // std::cout << "file found" << std::endl;

        // if the file open, close it
        if (this->_file.is_open())
            this->_file.close();
        
        this->_file.open(this->_filepath.c_str());
        
        // if file cannot open for some reason
        if (!this->_file.is_open())
            this->_tryFileStatus = -1;

        return;
    }

    // default file not found
    this->_tryFileStatus = -1;
}

// [ON PROCESS]
void HttpHandler::handle_cgi(void)
{
    std::cout << S_INFO << "Processing CGI: " << this->_url << S_END;

    // === specify CGI type === //
    std::string cgiExtension;
    cgiExtension.append(this->_url.substr(this->_url.rfind(".")));
    std::cout << S_DEBUG << "extension: " << cgiExtension << S_END;

    if (cgiExtension.compare(".sh") != 0 && cgiExtension.compare(".py") != 0)
    {
        std::cout << S_INFO <<  "CGI Not support type" << S_END;
        set_res_status(501, "Not Implemented");
        return;
    }

    // === Create the argv for execve === //
    this->_cgipath.append(".");
    this->_cgipath.append(this->_url);
    // std::cout << "cgi path : "  << this->_cgipath << std::endl;

    // check that there is a cgi file and executable
    if (access(this->_cgipath.c_str(), F_OK) == -1)
    {
        std::cout << S_WARNING << "File does not exist" << S_END;
        set_res_status(404, "Not Found");
        return;
    }

    // if the file was not permit to run
    if (access(this->_cgipath.c_str(), X_OK) == -1) {
        std::cout << S_WARNING << "Permission required for the file" << S_END;
        set_res_status(403, "Forbidden");
        return;
    }

    // === create the environment === //
    std::vector<const char *> env;

    env.push_back("PATH=/bin:/usr/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/sbin");

    std::string host("SERVER_NAME=127.0.0.1"); //========= hard code
    env.push_back(host.c_str());

    std::string content_length("CONTENT_LENGTH=");
    env.push_back(content_length.append(int_to_string(this->_body.size())).c_str());

    std::string contentType("CONTENT_TYPE=");
    contentType.append(this->_parameter["Content-Type"]);
    env.push_back(contentType.c_str());

    env.push_back("GATEWAT_INTERFACE=CGI/1.1");

    std::string path_info("PATH_INFO=");
    env.push_back(path_info.append(this->_url).c_str());

    std::string reqMethod("REQUEST_METHOD=");
    reqMethod.append(this->_method);
    env.push_back(reqMethod.c_str());

    // === GET : put it in the query string === //
    std::string query_string("QUERY_STRING=");
    env.push_back(query_string.append(this->_queryString).c_str());
    std::cout << "query string for get: " << query_string << std::endl;

    // === NULL terminate : End part of environment=== //
    env.push_back(NULL);

    this->_isCGI = 1; // hard code

    if (pipe(this->_to_cgi_fd) == -1 || pipe(this->_from_cgi_fd) == -1)
    {
        perror("pipe");
        set_res_status(500, "Internal Server Error");
        return;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        // error case
        set_res_status(500, "Internal Server Error");
        return;
    }
    else if (pid == 0)
    {
        // Child process

        close(this->_to_cgi_fd[1]);   // close to cgi write end
        close(this->_from_cgi_fd[0]); // close from cgi read end

        dup2(this->_to_cgi_fd[0], STDIN_FILENO);    // dup read-to to stdno
        dup2(this->_from_cgi_fd[1], STDOUT_FILENO); // dup write-from to stdout

        close(this->_to_cgi_fd[0]);   // close original
        close(this->_from_cgi_fd[1]); // close original

        // close(this->_fd); // close the server fd // not sure
        for (int i = 0; i < (int)this->_server.size(); i++)
        {
            if (this->_server[i].get_fd() > 0)
                close(this->_server[i].get_fd());
        }

        // === create parameter for execve === //
        std::vector<const char *> argv;
        argv.push_back(this->_cgipath.c_str());
        argv.push_back(NULL);

        // execve
        execve(argv[0], const_cast<char *const *>(argv.data()), const_cast<char *const *>(env.data()));
        std::cout << "[ERROR]: execve error => " << strerror(errno) << std::endl;
        set_res_status(500, "CGI FAILED");
    }
    else
    {
        // Parent process

        close(this->_to_cgi_fd[0]);   // close to cgi write end
        close(this->_from_cgi_fd[1]); // close from cgi read end

        // write post data to pipe
        write(this->_to_cgi_fd[1], this->_body.c_str(), this->_body.size());
        close(this->_to_cgi_fd[1]); // close when write complete

        std::vector<char> bf(BUFFER_SIZE);
        int bytesRead;

        // wait for CGI writing back to server, push it in the _cgiResBody
        while (1)
        {

            bytesRead = read(this->_from_cgi_fd[0], bf.data(), BUFFER_SIZE);
            // std::cout << bf.data() << std::endl;

            if (bytesRead == 0)
            {
                std::cout << "[CGI]: end of file detecting" << std::endl;
                set_res_status(200, "CGI OK");
                break;
            }

            if (bytesRead == -1)
            {
                std::cout << "[CGI]: error reading for CGI output" << std::endl;
                this->_isCGI = 0;
                set_res_status(500, "CGI FAILED");
                break;
            }

            std::cout << "[CGI]: reading " << bytesRead << std::endl;
            // std::cout << bf.data() << std::endl;

            this->_res.append(bf.data(), bytesRead); // fixed this to add only what read
        }

        close(this->_from_cgi_fd[0]); // close read end pipe when completed

        // std::cout << "===== CGI OUTPUT =====" << std::endl;
        // std::cout << this->_res << std::endl;
        // std::cout << "===== CGI OUTPUT =====" << std::endl;

        waitpid(pid, NULL, 0); // wait for child to finish

    }

    this->_status = CONTENT_PHASE;

    // check that they are well-format html like it should be ended with </html>
    if (this->_res.rfind("</html>") == std::string::npos) {
        std::cout << YELLOW << "[CGI]: Something wrong for CGI output" << S_END;
        this->_isCGI = 0;
        set_res_status(500, "Internal Server Error");
        return ;
    }

    this->_tryFileStatus = -1;
    this->_resContentType = std::string("text/html");
    set_res_status(200, "CGI OK");
}

// [OK]
void HttpHandler::uploading_task(void)
{
    // only support for POST and multipart/formdata
    // protect case // not support other entype
    if (this->_method.compare("POST") != 0 || this->_postType != FORMDATA)
    {
        set_res_status(422, "Unprocessable Entity");
        return;
    }

    // check if directory exist
    std::string uploadPath;

    std::size_t startIndex, length;
    startIndex = this->_location.find("uploadPath:");

    // if no uploadPath found, use fall back to root
    if (startIndex == std::string::npos)
        uploadPath.append(this->_root);
    else
    {
        length = this->_location.find(";", startIndex + 1) - startIndex;
        uploadPath.append(this->_location.substr(startIndex + 11, length - 11));
        if (uploadPath[0] == '/')
            uploadPath.erase(0, 1); // remove first /
    }
    
    // std::cout << "uploadpath: " << uploadPath << std::endl;

    // check directory is exist
    if (access(uploadPath.c_str(), F_OK) == -1) {
        perror("upload");
        std::cout << "could be " << std::endl;
        set_res_status(404, "Not Found");
        return ;
    }

    // check directory is writable
    if (access(uploadPath.c_str(), W_OK) == -1) {
        set_res_status(403, "Forbidden");
        return ;
    }

    // std::cout << "============== start of body ==============" << std::endl;
    // std::cout << this->_body << std::endl;
    // std::cout << "============== end of body ==============" << std::endl;

    std::size_t sindex = 0, eindex = 0;
    std::string boundary(this->_parameter["boundary"]);

    while (1)
    {
        // grab each block first
        sindex = this->_body.find(boundary, sindex);
        if (sindex == std::string::npos)
            break;

        eindex = this->_body.find(boundary, sindex + boundary.size()); // eindex cant change as it would set the next position
        if (eindex == std::string::npos)
            break;

        length = eindex - sindex - boundary.size();

        std::string block(this->_body.substr(sindex + 2 + boundary.size(), length - 4)); // minus \r\n at the end of first line

        // std::cout << "============== start of block ==============" << std::endl;
        // std::cout << block << std::endl;
        // std::cout << "============== end of block ==============" << std::endl;

        // after we get the block, check the content-disposition and find the filename

        sindex = block.find("Content-Disposition:");

        if (sindex == std::string::npos) // =================== in case no disposition attiribute
            continue;

        length = block.find("\r\n", sindex + 1) - sindex;
        std::string disposition(block.substr(sindex, length));
        // std::cout << disposition << std::endl;

        sindex = disposition.find("filename=\"");
        if (sindex == std::string::npos) // =================== in case no filename attribute
            continue;

        length = disposition.find("\"", sindex + 10) - sindex - 10; // 1 from the from and 1 from the back
        std::string filename(disposition, sindex + 10, length);

        // if filename is null, mean nothing pass in
        if (filename.size() == 0) {
            set_res_status(400, "Bad Request");
            return ;
        }

        // get the file data
        sindex = block.find("\r\n\r\n");
        if (sindex == std::string::npos) // ==================== in case of not found \r\n\r\n // imposible case
            continue;

        // std::cout << block.substr(sindex + 4) << std::endl;

        // open the file and write to it
        filename.insert(0, "/");
        std::size_t uindex = this->_location.find("uploadPath:");
        if (uindex == std::string::npos) // ============= if no uploadPathh found, use fall back to root
            filename.insert(0, this->_root);
        else
        {
            length = this->_location.find(";", uindex + 1) - uindex - 11;
            filename.insert(0, this->_location.substr(uindex + 11, length));
            filename.erase(0, 1); // remove first /
        }

        // std::cout << "filename: " << filename << std::endl;

        // check if filename is no permission to overwrite
        // if (access(filename.c_str(), W_OK) == -1) {
        //     std::cout << S_WARNING << "files cannot be overwriten" << S_END;
        //     set_res_status(403, "Forbidden");
        //     this->_status = CONTENT_PHASE;
        //     return ;
        // }

        std::ofstream file(filename.c_str(), std::ios::binary);

        // if file cannot be open for some reason
        if (!file.is_open())
        {
            std::cout << S_WARNING << strerror(errno) << S_END;
            set_res_status(500, "Internal Server Error");
            this->_status = CONTENT_PHASE;
            // sindex = eindex - 1; // return back 1 char
            // continue;
            return ;
        } else {
            file << block.substr(sindex + 4);
            file.close();
            std::cout << S_DEBUG << "File save successfully" << S_END;
            set_res_status(201, "Created");
            this->_tryFileStatus = -1;
            return ;
        }

        sindex = eindex - 1; // return back 1 char
        // to next block
    }

    this->_status = CONTENT_PHASE;
    return;
}

// [TODO]
void HttpHandler::content_builder(void)
{
    std::string fileData(""); // for other response that not CGI
    
    // if not cgi
    if (this->_isCGI != 1)
    {
        if (this->_tryFileStatus != -1)
        {
            // read from the file, should be at try_file na
            fileData = std::string(std::istreambuf_iterator<char>(this->_file), std::istreambuf_iterator<char>());
        }
        else if (this->_isRedirection != 1)
        {
            fileData.append(int_to_string(this->_resStatusCode));
            fileData.append(" : ");
            fileData.append(this->_resStatusText);
        }

        // close the file if it open
        if (this->_file.is_open())
            this->_file.close();

        if (this->_fileSize == 0)
            this->_fileSize = fileData.size();

        // std::cout << "file size: " << this->_fileSize << std::endl;
    }

    // create response header
    this->_response.clear();

    // create first line
    this->_response.append("HTTP/1.1 ");
    this->_response.append(int_to_string(this->_resStatusCode));
    this->_response.append(" ");
    this->_response.append(this->_resStatusText);
    this->_response.append("\r\n");

    // add Content-disposition for all that not html
    if (this->_resContentType.compare("text/html") != 0 && this->_resStatusCode == 200)
    {
        this->_response.append(create_res_attribute("Content-Disposition", "attachment"));
    };

    if (this->_isRedirection == 1)
    {
        this->_response.append(create_res_attribute("Location", this->_filepath));
        this->_response.append("\r\n\r\n");
    }
    else if (this->_isCGI == 1)
    {
        this->_response.append(create_res_attribute("Content-Length", int_to_string(this->_res.size() - this->_res.find("<html>"))));
        this->_response.append(this->_res);
    }
    else
    {
        this->_response.append(create_res_attribute("Content-Length", int_to_string(this->_fileSize)));
        this->_response.append(create_res_attribute("Content-Type", this->_resContentType));
        this->_response.append("\r\n");
        this->_response.append(fileData);
    }

    this->_status = SENDING;

    // std::cout << "===================" << std::endl;
    // std::cout << this->_response << std::endl;
    // std::cout << "===================" << std::endl;
}

void HttpHandler::sending(void) {

    int sentByte;

    if (this->_response.size() - this->_bytesSent < BUFFER_SIZE)
        sentByte = send(this->_fd, this->_response.c_str() + this->_bytesSent, this->_response.size() - this->_bytesSent, 0);
    else
        sentByte = send(this->_fd, this->_response.c_str() + this->_bytesSent, BUFFER_SIZE, 0);

    this->_bytesSent += sentByte;

    // std::cout << "sending: " << this->_bytesSent << "/" << this->_response.size() << std::endl;
    
    if (sentByte == -1) {
        std::cout << S_ERROR << "send failed [" << this->_fd << "]" << S_END;
        this->_status = CLOSED;
        return ;
    }

    if (sentByte == 0) {
        std::cout << S_WARNING << "[" << this->_fd << "] connection was closed" << S_END;
        this->_status = CLOSED;
        return ;
    }

    if (this->_bytesSent >= this->_response.size()) {
        std::cout << "completed: " << this->_bytesSent << "/" << this->_response.size() << std::endl;
        this->_status = COMPLETED_PHASE;
        return ;
    }

}

// [OK]
void HttpHandler::parsing_error_code(std::string str)
{
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
}

// Utils
void HttpHandler::remove_white_space(std::string &input)
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

int HttpHandler::string_to_int(std::string str)
{
    std::stringstream ss(str);
    int output;
    ss >> output;
    return (output);
}

std::size_t HttpHandler::string_to_size(std::string str)
{
    std::stringstream ss(str);
    std::size_t output;
    ss >> output;
    return (output);
}

std::string HttpHandler::int_to_string(int n)
{
    std::stringstream ss;
    ss << n;
    std::string output(ss.str());
    return (std::string(output));
}

std::string HttpHandler::create_res_attribute(std::string key, std::string value)
{
    std::string attr;
    attr.append(key);
    attr.append(": ");
    attr.append(value);
    attr.append("\r\n");
    return (std::string(attr));
}