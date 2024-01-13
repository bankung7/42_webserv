#include "HttpHandler.hpp"

// [ON PROCESS]
void HttpHandler::handle_cgi(void)
{
    std::cout << S_INFO << "[" << this->_fd << "][CGI] Processing => " << this->_url << S_END;

    // === specify CGI type === //
    std::string cgiExtension;
    cgiExtension.append(this->_url.substr(this->_url.rfind(".")));

    // std::cout << S_DEBUG << "extension: " << cgiExtension << S_END;
    if (cgiExtension.compare(".sh") != 0 && cgiExtension.compare(".py") != 0)
    {
        std::cout << S_INFO << "[" << this->_fd << "][CGI] Not support type" << S_END;
        set_res_status(501, "Not Implemented");
        return;
    }
    // =================>


    // === Create the argv for execve === //
    this->_cgipath.append(".");
    this->_cgipath.append(this->_url);
    // std::cout << "cgi path : "  << this->_cgipath << std::endl;

    // check that there is a cgi file and executable
    if (access(this->_cgipath.c_str(), F_OK) == -1)
    {
        std::cout << S_INFO << "[" << this->_fd << "][CGI] File does not exist" << S_END;
        set_res_status(404, "Not Found");
        return;
    }

    // if the file was not permit to run
    if (access(this->_cgipath.c_str(), X_OK) == -1)
    {
        std::cout << S_INFO << "[" << this->_fd << "][CGI] Permission required for the file" << S_END;
        set_res_status(403, "Forbidden");
        return;
    }
    // ===================>

    // === create the environment === //
    this->_cgiEnv.push_back("PATH=/bin:/usr/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/sbin");

    std::string host("SERVER_NAME=127.0.0.1"); //========= hard code
    this->_cgiEnv.push_back(host.c_str());

    std::string content_length("CONTENT_LENGTH=");
    this->_cgiEnv.push_back(content_length.append(int_to_string(this->_body.size())).c_str());

    std::string contentType("CONTENT_TYPE=");
    contentType.append(this->_parameter["Content-Type"]);
    this->_cgiEnv.push_back(contentType.c_str());

    this->_cgiEnv.push_back("GATEWAT_INTERFACE=CGI/1.1");

    std::string path_info("PATH_INFO=");
    this->_cgiEnv.push_back(path_info.append(this->_url).c_str());

    std::string reqMethod("REQUEST_METHOD=");
    reqMethod.append(this->_method);
    this->_cgiEnv.push_back(reqMethod.c_str());

    // === GET : put it in the query string === //
    std::string query_string("QUERY_STRING=");
    this->_cgiEnv.push_back(query_string.append(this->_queryString).c_str());
    // std::cout << "query string for get: " << query_string << std::endl;

    // === NULL terminate : End part of environment=== //
    this->_cgiEnv.push_back(NULL);
    // ====================>


    // === create parameter for execve === //
    std::vector<const char *> argv;
    argv.push_back(this->_cgipath.c_str());
    argv.push_back(NULL);
    // ====================>


    // Set the basic information for CGI which will be using in the loop
    this->_resContentType = std::string("text/html");
    this->_tryFileStatus = -1;
    this->_isCGI = 1;
    // =============================>

    // give more timeout for cgi
    this->_timeout += 5;

    if (pipe(this->_to_cgi_fd) == -1 || pipe(this->_from_cgi_fd) == -1)
    {
        set_res_status(500, "Internal Server Error");
        return;
    }

    this->_pid = fork();

    if (this->_pid < 0)
    {
        // error case
        close(this->_to_cgi_fd[0]);
        close(this->_to_cgi_fd[1]);
        close(this->_from_cgi_fd[0]);
        close(this->_from_cgi_fd[1]);
        set_res_status(500, "Internal Server Error");
        return;
    }
    // Child process
    else if (this->_pid == 0)
    {
        close(this->_to_cgi_fd[1]);   // close to cgi write end
        close(this->_from_cgi_fd[0]); // close from cgi read end

        dup2(this->_to_cgi_fd[0], STDIN_FILENO);    // dup read-to to stdno
        dup2(this->_from_cgi_fd[1], STDOUT_FILENO); // dup write-from to stdout

        close(this->_to_cgi_fd[0]);   // close original
        close(this->_from_cgi_fd[1]); // close original

        // execve
        int status = execve(argv[0], const_cast<char *const *>(argv.data()), const_cast<char *const *>(this->_cgiEnv.data()));

        // if execve failed, exit(1)
        // std::cout << S_WARNING << "execve error => " << strerror(errno) << S_END;
        exit(status);
    }
    // Parent process
    else
    {

        close(this->_to_cgi_fd[0]);   // close to cgi write end
        close(this->_from_cgi_fd[1]); // close from cgi read end

        // set _to_cgi_fd[1] as non block
        if (fcntl(this->_to_cgi_fd[1], F_SETFL, O_NONBLOCK) == -1)
        {
            kill(this->_pid, SIGINT); // kill the child process;
            close(this->_to_cgi_fd[1]);
            std::cout << S_WARNING << "[" << this->_fd << "][CGI] set non block failed" << S_END;
            set_res_status(500, "Internal Server Error");
            this->_status = CONTENT_PHASE;
            return;
        }

        if (fcntl(this->_from_cgi_fd[0], F_SETFL, O_NONBLOCK) == -1)
        {
            kill(this->_pid, SIGINT); // kill the child process;
            close(this->_from_cgi_fd[0]);
            std::cout << S_WARNING << "[" << this->_fd << "][CGI] set non block failed" << S_END;
            set_res_status(500, "Internal Server Error");
            this->_status = CONTENT_PHASE;
            return;
        }

        // add this to epoll
        this->_status = CGI_OUT;
        return;
    }
}

void HttpHandler::cgi_writing(void) {

    // if nothing to send, just close the pipe
    if (this->_body.size() == 0) {
        // std::cout << S_DEBUG << "Null body" << S_END;
        this->_status = CGI_IN;
        return ;
    }

    int bytesSent;

    if (this->_body.size() - this->_toCgiBytes < BUFFER_SIZE)
        bytesSent = write(this->_to_cgi_fd[1], this->_body.c_str() + this->_toCgiBytes, this->_body.size() - this->_toCgiBytes);
    else
        bytesSent = write(this->_to_cgi_fd[1], this->_body.c_str() + this->_toCgiBytes, BUFFER_SIZE);

    // [TODO]
    if (bytesSent == -1) {
        std::cout << S_WARNING << "[CGI][" << this->_to_cgi_fd[1] << "] error while sending to CGI" << S_END;
        set_res_status(500, "Internal Server Error");
        close(this->_to_cgi_fd[1]);
        close(this->_from_cgi_fd[1]);
        this->_parameter["Connection"] = std::string("closed");
        return ;
    }

    // in case of cannot write for a while
    if (bytesSent == 0) {
        std::cout << S_WARNING << "[CGI][" << this->_to_cgi_fd[1] << "] cannot write at the moment, will try later" << S_END;
        return ;
    }

    this->_toCgiBytes += bytesSent;

    if (this->_toCgiBytes >= this->_body.size()) {
        this->_status = CGI_IN;
        std::cout << S_DEBUG << "[" << this->_fd << "][CGI][" << this->_to_cgi_fd[1] << "] writing completed " << this->_toCgiBytes << "/" << this->_body.size() << S_END;
        return ;
    }

}

void HttpHandler::cgi_reading(void) {

    std::vector<char> bf(BUFFER_SIZE);
    int bytesRead;

    bytesRead = read(this->_from_cgi_fd[0], bf.data(), BUFFER_SIZE);

    // std::cout << S_DEBUG << "[CGI][" << this->_from_cgi_fd[0] << "] read :" << bytesRead << S_END;

    // error case
    if (bytesRead == -1) {
        set_res_status(500, "Internal Server Error");
        std::cout << S_WARNING << "[CGI][" << this->_from_cgi_fd[0] << "] error while reading from CGI" << S_END;
        this->_cgiStatus = ERROR;
        set_res_status(500, "Internal Server Error");
        this->_parameter["Connection"] = std::string("closed");
        return ;
    }

    // // if EOF found, this will found in EPOLLHUP
    // if (bytesRead == 0) {
    //     // [TODO]: check what status of cgi return
    //     waitpid(this->_pid, NULL, 0);

    //     // check if well-formed received
    //     if (this->_res.rfind("</html>") == std::string::npos) {
    //         std::cout << S_WARNING << "CGI output failed" << S_END;
    //         set_res_status(500, "Internal Server Error");
    //         this->_parameter["Connection"] = std::string("closed");
    //         return ;
    //     }

    //     // assume ok
    //     this->_resContentType = std::string("text/html");
    //     this->_tryFileStatus = -1;
    //     this->_isCGI = 1;
    //     this->_status = CONTENT_PHASE;
    //     set_res_status(200, "OK");
    //     return ;
    // }

    // push to res
    this->_res.append(bf.data(), bytesRead);

    std::cout << S_INFO << "[" << this->_fd << "][CGI][" << this->_from_cgi_fd[0] << "] Status[" << this->_status << "] read[" << bytesRead << "]" << S_END;

}

int HttpHandler::cgi_get_from_fd(void) {
    return (this->_from_cgi_fd[0]);
}

int HttpHandler::cgi_get_to_fd(void) {
    return (this->_to_cgi_fd[1]);
}

pid_t HttpHandler::cgi_get_pid(void) {
    return (this->_pid);
}

int HttpHandler::cgi_get_status(void) {
    return (this->_cgiStatus);
}
