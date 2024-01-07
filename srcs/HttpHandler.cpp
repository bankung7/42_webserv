#include "HttpHandler.hpp"

HttpHandler::HttpHandler(void)
{
}

HttpHandler::HttpHandler(int fd) : _fd(fd), _status(READING), _serverIndex(-1)
{

    this->_parameter["Host"] = std::string("");
    this->_parameter["Content-Type"] = std::string("");
    this->_parameter["Content-Length"] = std::string("0");
    this->_parameter["boundary"] = std::string("");
    this->_parameter["Connection"] = std::string("keep-alive"); // default to keep-alive
    this->_parameter["Keep-Alive"] = std::string("");

    this->_reqContentLength = 0;
    this->_readState = 0;
    this->_status = NOTSTART;

    std::time(&this->_timeout); // default
    this->_timeout += KEEP_ALIVE_TIME_OUT;

    this->_postType = 0;
    this->_maxClientBodySize = std::numeric_limits<std::size_t>::max(); // if not specified

    // cgi part
    this->_isCGI = 0;
    this->_queryString = std::string("");

    this->_tryFileStatus = 0;

    this->_isDirectory = 0;
    this->_isRedirection = 0;
    this->_isAutoIndex = 0;
    this->_isIndex = 0;
    this->_fileSize = 0;

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

    std::cout << "[DEBUG]: " << this->_filepath << std::endl;

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

// handle request
void HttpHandler::handle_request(void)
{

    std::vector<char> bf(BUFFER_SIZE);
    int bytesRead;

    bf.clear();
    bytesRead = recv(this->get_fd(), bf.data(), BUFFER_SIZE, 0);

    std::cout << "[DEBUG]: Request Bytes read: " << bytesRead << std::endl;

    if (bytesRead < 0)
    {
        // error case
        std::cout << RED << "[WARNING]: Client [" << this->_fd << "] has something error while reading" << C_RESET << std::endl;
        set_status(CLOSED); // it might be set to other error
    }
    else if (bytesRead == 0)
    {
        // client closed the connection
        std::cout << YELLOW << "[INFO]: Client [" << this->_fd << "]closed the connection" << C_RESET << std::endl;
        set_status(CLOSED);
    }
    else
    {
        // normal case, try to parsing the request and check that all message received

        if (this->_readState == 0)
            this->_req.append(bf.data(), bytesRead); // write to (res) header
        else
            this->_body.append(bf.data(), bytesRead); // write to body

        // when reading to body [POST] only
        if (this->_readState == 1)
        {
            // if completed, to epollout
            if (this->_body.size() == this->_reqContentLength)
            {
                // std::cout << "size: " << this->_reqContentLength << " " << this->_body.size() << std::endl;
                this->_readState = 2;
                this->_status = POST_READ_PHASE;
                return;
            }

            // if not completed, queue in epollin
            return;
        }

        // check \r\n\r\n
        if (this->_readState == 0 && this->_req.find("\r\n\r\n") != std::string::npos)
        {
            // std::cout << "==== State 0 ====" << std::endl;
            // split after \r\n\r\n to body if exist, and remove it for req
            this->_body.append(this->_req.substr(this->_req.find("\r\n\r\n") + 4));
            this->_req.erase(this->_req.find("\r\n\r\n"));

            this->_readState = 1;
            this->_status = READING;

            // set up header parameter
            setup_header();

            if (this->_body.size() >= this->_reqContentLength)
            { // check later
                std::cout << "[DEBUG]: Request POST size: " << this->_reqContentLength << " " << this->_body.size() << std::endl;
                this->_readState = 2;
                this->_status = POST_READ_PHASE;
                return;
            }

            return;
        }
    }
}

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
                std::cout << "Content-Length: " << this->_reqContentLength << std::endl;
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
        this->_parameter["Content-Type"].append("text/html");

}

void HttpHandler::handle_response(void)
{
    // Assign server block
    assign_server_block();

    // match location
    assign_location_block();

    // create response
    create_response();

    // try file
    try_file();

    // content builder
    content_builder();
}

void HttpHandler::assign_server_block(void)
{
    this->_status = FIND_SERVER_PHASE;

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
            this->_status = FIND_LOCATION_PHASE;
            return;
        }

        // if the port and  server name match
        if (this->_server[i].has_server_name(this->_parameter["Host"]) == 1)
        {
            // std::cout << this->_parameter["Host"] << " " << this->_server[i].get_server_name(this->_parameter["Host"]) << std::endl;
            this->_serverIndex = i;
            this->_status = FIND_LOCATION_PHASE;
            return;
        }

    }

    set_res_status(404, "Not Found");
    this->_parameter["Connection"] = "closed";
    this->_status = CONTENT_PHASE; // as no servername match
}

void HttpHandler::assign_location_block(void)
{

    if (this->_status != FIND_LOCATION_PHASE)
        return ;

    // get the location block
    this->_loc.append(this->_server[this->_serverIndex].best_match_location(this->_url));
    this->_location.append(this->_server[this->_serverIndex].get_location(this->_loc));

    this->_status = CREATE_RESPONSE_PHASE;

    // std::cout << this->_loc << std::endl;
    // std::cout << this->_location << std::endl;
}

void HttpHandler::create_response(void)
{

    if (this->_status != CREATE_RESPONSE_PHASE)
        return ;

    std::size_t startIndex, length;

    startIndex = 0;

    // Set errorcode into the map
    while (1)
    {
        startIndex = this->_location.find("error_page:");
        if (startIndex == std::string::npos)
            break;

        length = this->_location.find(";", startIndex + 1) - startIndex;
        parsing_error_code(std::string(this->_location.substr(startIndex + 11, length - 11)));
        this->_location.insert(startIndex + 10, "R");
    }

    // set root == MANDATORY
    startIndex = this->_location.find("root:");
    if (startIndex == std::string::npos)
    {
        // ================ this will fall down from the server itself
        this->_root = std::string(this->_server[this->_serverIndex].get_root()); // set root
    }
    else
    {
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

    if (attribute.find(this->_method.c_str(), startIndex) == std::string::npos)
    {
        // std::cout << "Request Method " << this->_method << std::endl;
        // std::cout << "Allowed Method " << attribute << std::endl;
        std::cout << "\033[1;31mRequest " << this->_method << " method is not allowed\033[0m" << std::endl;
        set_res_status(405, "METHOD NOT ALLOWED");
        this->_status = TRY_FILE_PHASE;
        return;
    }

    // check client limit == OPTIONAL
    startIndex = this->_location.find("client_max_body_size:");
    length = this->_location.find(";", startIndex + 1) - startIndex - 21;

    // std::cout << this->_location.substr(startIndex + 21, length) << std::endl;

    if (startIndex != std::string::npos)
    {
        this->_maxClientBodySize = string_to_size(this->_location.substr(startIndex + 21, length));
        // std::cout << "max size: " << this->_maxClientBodySize << std::endl;
        // std::cout << "body legnth: " << this->_reqContentLength << std::endl;
        if (this->_maxClientBodySize < this->_reqContentLength)
        {
            std::cout << "\033[1;33m[WARNING]: File is too large than server limit " << this->_maxClientBodySize << "\033[0m" << std::endl;
            set_res_status(413, "Content Too Large");
            this->_status = TRY_FILE_PHASE;
            return;
        }
    }

    // Redirection == OPTIONAL
    if (this->_location.find("return:") != std::string::npos)
    {

        startIndex = this->_location.find("return: ");
        length = this->_location.find(";", startIndex + 1) - startIndex - 8;

        // std::cout << "\033[1;31mRedirection case : " << this->_location.substr(startIndex + 8, length) << "\033[0m" << std::endl;

        // split to code and url or text
        std::stringstream redirection(this->_location.substr(startIndex + 8, length));
        std::string attr;

        // get the status code
        std::getline(redirection, attr, ' ');
        int code = string_to_int(attr);

        // std::cout << attr << std::endl;

        // relocation, get url
        if (code >= 300 && code <= 399)
        {

            this->_isRedirection = 1;
            this->_filepath.clear();

            std::getline(redirection, attr);
            this->_filepath = std::string(attr);
            set_res_status(code, "MOVED");
            this->_status = CONTENT_PHASE;
            return;
        }

        // other get text to set response
        std::getline(redirection, attr);
        attr.erase(0, 1); // remove " at beginning
        attr.erase(attr.size() - 1); // remove " at the end
        this->_isRedirection = 0;
        set_res_status(code, attr);
        return;
    }

    // put the filename in the path
    // so the filepath should be root + url
    this->_filepath.append(this->_url);
    this->_status = TRY_FILE_PHASE;

    // === GET method with query ? === //
    // put query part into _queryString
    if (this->_method.compare("GET") == 0 && this->_url.find("?") != std::string::npos)
    {
        this->_queryString.append(this->_url.substr(this->_url.find("?") + 1));
        this->_url.erase(this->_url.find("?"));
    }

    // for CGI == OPTIONAL =======================>
    if (std::string("/cgi-bin/").find(this->_loc.c_str(), 0, 9) == 0)
    {
        // std::cout << "CGI requesting" << std::endl;
        // accept only text/html and urlencoded

        if (this->_parameter["Content-Type"].compare("text/html") != 0
            && this->_parameter["Content-Type"].compare("application/x-www-form-urlencoded") != 0)
        {
            this->_isCGI = 0;
            set_res_status(403, "Not Accept multipart/formdata");
            this->_status = TRY_FILE_PHASE;
            return;
        }

        this->_isCGI = 1;
        handle_cgi();
        this->_status = CONTENT_PHASE;
        return;
    }

    // file upload
    if (this->_location.find("allowedFileUpload:") != std::string::npos)
    {

        this->_status = CONTENT_PHASE;
        if (this->_location.find("allowedFileUpload:yes;") == std::string::npos)
        {
            std::cout << "\033[1;31Uploadfile is not allowed : " << std::string(this->_location.substr(startIndex + 7, length)) << "\033[0m" << std::endl;
            set_res_status(403, "Forbidden");
            return;
        }
        this->_resContentType = std::string("text/html");
        uploading_task();
        return;
    }

    // check if directory or not
    stat(this->_filepath.c_str(), &this->_fileInfo);

    switch (this->_fileInfo.st_mode & S_IFMT)
    {
    case S_IFDIR: // if it is the directory
        this->_isDirectory = 1;
        break;
    default: // if nothing else
        break; // not support
    }

    // is directory == OPTIONAL
    if (this->_isDirectory == 1)
    {
        // set that directory can't be downloaded
        this->_resContentType = std::string("text/html");

        // in case match location is directory but not / at the end
        if (this->_filepath[this->_filepath.size() - 1] != '/')
            this->_filepath.append("/");

        // index page accepted == OPTIONAL
        if (this->_location.find("index:") != std::string::npos)
        {
            // std::cout << indexPage << " is provided for this request" << std::endl;
            this->_isIndex = 1;
            startIndex = this->_location.find("index:");
            length = this->_location.find(";", startIndex + 1) - startIndex;
            this->_filepath.append(this->_location.substr(startIndex + 6, length - 6));
            this->_status = TRY_FILE_PHASE;
            return;
        }

        // is autoindex == OPTIONAL, if off, don't set this in the structure
        // this might use cgi to generate the file into prepared folder
        if (this->_location.find("autoIndex:on;") != std::string::npos)
        {
            std::cout << "Index Page requested" << std::endl;
            this->_isAutoIndex = 1;
            // this->_filepath.append("indexofpage.html");

            // ====================================================> will use cgi to build the index pag
            // assume that be here with GET
            if (this->_method.compare("GET") == 0)
            {
                // put the url to query as it will be path
                this->_queryString = std::string("");
                this->_queryString.append("root=");
                this->_queryString.append(this->_root);
                this->_queryString.append("&");
                this->_queryString.append("url=");
                this->_queryString.append(this->_url.substr(1, this->_url.size() - 1));

                this->_url.clear(); // remove the url, replace with the path of cgi
                this->_url.append("/cgi-bin/directory_listing.py");
                this->_isCGI = 1;

                handle_cgi();

                this->_tryFileStatus = -1;
                this->_status = CONTENT_PHASE;
                return ;
            }

            this->_status = TRY_FILE_PHASE;
            return;
        }

        // in case of no index and autoindex, set index.html as default
        this->_filepath.append("index.html");

    }

}

void HttpHandler::handle_cgi(void)
{
    std::cout << "[CGI]: Processing CGI: " << this->_url << std::endl;

    // === specify CGI type === //
    std::string cgiExtension;
    cgiExtension.append(this->_url.substr(this->_url.rfind(".")));
    // std::cout << "extension: " << cgiExtension << std::endl;

    if (cgiExtension.compare(".sh") != 0 && cgiExtension.compare(".py") != 0)
    {
        std::cout << "[CGI]: Not support type" << std::endl;
        set_res_status(404, "CGI NOT SUPPORT FILE");
        this->_status = TRY_FILE_PHASE;
        return;
    }

    // === Create the argv for execve === //
    this->_cgipath.append(".");
    this->_cgipath.append(this->_url);
    // std::cout << "cgi path : "  << this->_cgipath << std::endl;

    // check that there is a cgi file and executable
    if (access(this->_cgipath.c_str(), F_OK) == -1)
    {
        std::cout << YELLOW << "[WARNING]: File does not exist" << C_RESET << std::endl;
        set_res_status(404, "CGI not found");
        this->_resContentType = std::string("text/html");
        this->_status = CONTENT_PHASE;
        this->_tryFileStatus = -1;
        this->_isCGI = 0;
        return;
    }

    // if the file was not permit to run
    if (access(this->_cgipath.c_str(), X_OK) == -1) {
        std::cout << YELLOW << "[WARNING]: Permission required for the file" << C_RESET << std::endl;
        set_res_status(403, "Forbidden");
        this->_resContentType = std::string("text/html");
        this->_status = CONTENT_PHASE;
        this->_tryFileStatus = -1;
        this->_isCGI = 0;
        return;
    }

    // === create the environment === //
    std::vector<const char *> env;

    env.push_back("PATH=/bin:/usr/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/sbin");

    std::string host("SERVER_NAME=127.0.0.1:8080"); //========= hard code
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
    // std::cout << "query string fro get: " << query_string << std::endl;

    // === NULL terminate : End part of environment=== //
    env.push_back(NULL);

    int to_cgi_fd[2];
    int from_cgi_fd[2];

    if (pipe(to_cgi_fd) == -1 || pipe(from_cgi_fd) == -1)
    {
        perror("pipe");
        set_res_status(500, "Failed to pipe");
        this->_status = TRY_FILE_PHASE;
        return;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        // error case
        perror("fork");
        set_res_status(500, "failed to fork");
        this->_status = TRY_FILE_PHASE;
        return;
    }
    else if (pid == 0)
    {
        // Child process

        close(to_cgi_fd[1]);   // close to cgi write end
        close(from_cgi_fd[0]); // close from cgi read end

        dup2(to_cgi_fd[0], STDIN_FILENO);    // dup read-to to stdno
        dup2(from_cgi_fd[1], STDOUT_FILENO); // dup write-from to stdout

        close(to_cgi_fd[0]);   // close original
        close(from_cgi_fd[1]); // close original

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
        this->_status = TRY_FILE_PHASE;
        set_res_status(500, "CGI FAILED");
    }
    else
    {
        // Parent process

        close(to_cgi_fd[0]);   // close to cgi write end
        close(from_cgi_fd[1]); // close from cgi read end

        // write post data to pipe
        write(to_cgi_fd[1], this->_body.c_str(), this->_body.size());
        close(to_cgi_fd[1]); // close when write complete

        std::vector<char> bf(BUFFER_SIZE);
        int bytesRead;

        // wait for CGI writing back to server, push it in the _cgiResBody
        while (1)
        {

            bytesRead = read(from_cgi_fd[0], bf.data(), BUFFER_SIZE);
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

        close(from_cgi_fd[0]); // close read end pipe when completed

        // std::cout << "===== CGI OUTPUT =====" << std::endl;
        // std::cout << this->_res << std::endl;
        // std::cout << "===== CGI OUTPUT =====" << std::endl;

        waitpid(pid, NULL, 0); // wait for child to finish

    }

    this->_status = CONTENT_PHASE;

    // check that they are well-format html like it should be ended with </html>
    if (this->_res.rfind("</html>") == std::string::npos) {
        std::cout << YELLOW << "[CGI]: Something wrong for CGI output" << C_RESET << std::endl;
        this->_isCGI = 0;
        set_res_status(500, "Internal Server Error");
        return ;
    }

    this->_tryFileStatus = -1;
    this->_resContentType = std::string("text/html");
    set_res_status(200, "CGI OK");
}

void HttpHandler::uploading_task(void)
{
    // protect case // not support other entype
    if (this->_method.compare("POST") != 0 || this->_postType != FORMDATA)
    {
        set_res_status(403, "NOT ALLOWED");
        return;
    }

    // only formdata accepted
    if (this->_postType == FORMDATA)
    {
        // check if directory exist
        std::string uploadPath;

        std::size_t upindex, uplength;
        upindex = this->_location.find("uploadPath:");

        // ============= if no uploadPath found, use fall back to root
        if (upindex == std::string::npos)
            uploadPath.insert(0, this->_root);
        else
        {
            // ===== found
            uplength = this->_location.find(";", upindex + 1) - upindex - 11;
            uploadPath.insert(0, this->_location.substr(upindex + 11, uplength));
            uploadPath.erase(0, 1); // remove first /
        }

        struct stat sb;
        stat(uploadPath.c_str(), &sb);

        // std::cout << uploadPath << std::endl;

        if ((sb.st_mode & S_IFMT) != S_IFDIR) {
            set_res_status(404, "No Directory Path Found");
            return ;
        }

        // std::cout << "============== start of body ==============" << std::endl;
        // std::cout << this->_body << std::endl;
        // std::cout << "============== end of body ==============" << std::endl;

        std::size_t sindex = 0, eindex = 0, length = 0;
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
            // std::cout << "filename: " << filename << std::endl;

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
            { // ===== found
                length = this->_location.find(";", uindex + 1) - uindex - 11;
                filename.insert(0, this->_location.substr(uindex + 11, length));
                filename.erase(0, 1); // remove first /
            }
            // std::cout << "filename: " << filename << std::endl;

            std::ofstream file(filename.c_str(), std::ios::binary);

            if (!file)
            { //================ cannot open the file
                // std::cout << filename << std::endl;
                std::cout << "[ERROR]: " << strerror(errno) << std::endl;
                set_res_status(403, "FORBIDDEN");
                this->_status = TRY_FILE_PHASE;
                sindex = eindex - 1; // return back 1 char
                continue;
            }

            file << block.substr(sindex + 4);
            file.close();
            std::cout << "[DEBUG]: File save successfully" << std::endl;
            set_res_status(200, "OK file saved");
            this->_status = CONTENT_PHASE;
            this->_tryFileStatus = 2;

            sindex = eindex - 1; // return back 1 char
            // to next block
        }

        this->_status = CONTENT_PHASE;
        return;
    }

    set_res_status(200, "OK");
    this->_status = CONTENT_PHASE;

}

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

    if (it == this->_errorCode.end())
    {
        // std::cout << "no match " << code << std::endl;
        this->_tryFileStatus = -1; // this will return system error page
        return ;
    }
    else
    {
        // guaruntee that custom error_page is exist before try_file
        // std::cout << "the default error page was setup" << std::endl;
        this->_filepath.clear();
        this->_filepath.append(this->_root);
        this->_filepath.append(this->_errorCode[code]);

        // check if the custom error page is exist
        if (access(this->_filepath.c_str(), F_OK) != -1)
        { 
            // file found
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
        std::cout << "error filepath: " << this->_filepath << std::endl;
    }
}

void HttpHandler::try_file(void)
{
    if (this->_status != TRY_FILE_PHASE)
        return;

    // this step is reached when the filepath isset, even in the error code
    // std::cout << "try file status " << this->_tryFileStatus << std::endl;

    std::cout << "[DEBUG]: try file: " << this->_filepath << std::endl;

    // if file request not found
    if (access(this->_filepath.c_str(), F_OK) == -1)
    {
        // std::cout << YELLOW << "[WARNING]: File does not exist" << std::endl;
        set_res_status(404, "File Not Found");
        return ;
    }

    // fall back from set default error page, if no then send text/plain
    // if (this->_tryFileStatus == -1)
    //     return;

    // if delete method
    if (this->_method.compare("DELETE") == 0)
    {
        // std::cout << "delete method" << std::endl;

        // check file permission
        if (access(this->_filepath.c_str(), W_OK) == -1) {
            std::cout << YELLOW << "[WARNING]: You dont have permission to delete the file" << std::endl;
            set_res_status(403, "Forbidden");
            return ;
        }

        int status = std::remove(this->_filepath.c_str());
        if (status != 0)
        { // ==================== thing went wrong for macos, it ask for permission to deleted
            std::cout << "[ERROR]: Permission denied" << std::endl;
            set_res_status(403, "NO PERMISSION");
            return;
        }

        set_res_status(200, "OK");
        this->_tryFileStatus = -1; // [TODO]
        return;
    }

    // get the file size
    stat(this->_filepath.c_str(), &this->_fileInfo);

    // check the permission
    if (access(this->_filepath.c_str(), R_OK) == -1) {
        // dont have permission to read it
        set_res_status(403, "No Permission");
        return ;
    }

    this->_file.open(this->_filepath.c_str(), std::ios::in);

    // check if the file cannot open for some reasone
    if (!this->_file.is_open()) {
        std::cout << YELLOW << "[WARNING]: The file cannot be open for some reason" << std::endl;
        set_res_status(404, "File cannot open");
        return ;
    }

    this->_fileSize = this->_fileInfo.st_size;
    set_res_content_type();
    if (this->_resStatusCode == 0)
        set_res_status(200, "OK");

    // std::cout << "page not found " << this->_resStatusCode << std::endl;
}

void HttpHandler::content_builder(void)
{

    std::string fileData(""); // for other response that not CGI
    
    // std::cout << "content phase" << std::endl;

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
    std::string response;
    response.clear();

    // create first line
    response.append("HTTP/1.1 ");
    response.append(int_to_string(this->_resStatusCode));
    response.append(" ");
    response.append(this->_resStatusText);
    response.append("\r\n");

    // add Content-disposition for all that not html
    if (this->_resContentType.compare("text/html") != 0 && this->_resStatusCode == 200)
    {
        // std::cout << "Response Content Type : " << this->_resContentType << std::endl;
        response.append(create_res_attribute("Content-Disposition", "attachment"));
    };

    if (this->_isRedirection == 1)
    {
        // response.append(create_res_attribute("Content-Type", "0"));
        response.append(create_res_attribute("Location", this->_filepath));
        response.append("\r\n\r\n");
    }
    else if (this->_isCGI == 1)
    {
        response.append(create_res_attribute("Content-Length", int_to_string(this->_res.size() - this->_res.find("<html>"))));
        response.append(this->_res);
    }
    else
    {
        response.append(create_res_attribute("Content-Length", int_to_string(this->_fileSize)));
        response.append(create_res_attribute("Content-Type", this->_resContentType));
        response.append("\r\n");
        response.append(fileData);
    }

    // std::cout << response << std::endl;

    int totalByte = response.size();
    // std::cout << "total size to send: " << totalByte << std::endl;
    // int sentByte = send(this->_fd, response.c_str(), totalByte, 0);

    int bytesSent = 0;
    int sentByte = 0;
    while (1) {

        if (totalByte - bytesSent < BUFFER_SIZE)
            sentByte = send(this->_fd, response.c_str() + bytesSent, totalByte - bytesSent, 0);
        else
            sentByte = send(this->_fd, response.c_str() + bytesSent, BUFFER_SIZE, 0);

        if (sentByte > 0) {
            bytesSent += sentByte;
        } else if (sentByte == -1) {
            // std::cout << "send failed will keep try sending" << std::endl;
        }

        if (bytesSent == totalByte)
            break ;
            
    }

    // if (sentByte == -1)
    //     std::cout << "\033[;31m"
    //               << "[ERROR]: Something wrong while sending"
    //               << "\033[0m" << std::endl;

    if (totalByte > bytesSent)
        std::cout << "\033[;31m"
                  << "[ERROR]: Sendin not whole file"
                  << "\033[0m" << std::endl;
    else
        std::cout << "\033[;32m"
                  << "[DEBUG]: Send completed " << totalByte << " to " << this->_fd << "\033[0m" << std::endl;

    this->_status = COMPLETE_PHASE;
}

// errorpage set
void HttpHandler::error_page_set(int error, std::string text)
{
    set_res_status(error, text);
    this->_filepath.clear();
    this->_filepath.append(this->_root);
    this->_filepath.append("/error/");
    this->_filepath.append(int_to_string(error));
    this->_filepath.append(".html");
    this->_filepath.erase(0, 1);
    std::cout << "error path " << this->_filepath << std::endl;
}

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