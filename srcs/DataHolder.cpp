#include "DataHolder.hpp"

DataHolder::DataHolder(void) {

}

DataHolder::DataHolder(std::vector<char> &request): _headerLength(0), _bodyLength(0) {

    parsing(request);

    // check the request
    std::cout << "============== Client Request ===============" << std::endl;
    std::cout << "Method: " << this->_method << std::endl;
    std::cout << "Resource: " << this->_resource << std::endl;
    std::cout << "Version: " << this->_cversion << std::endl;
    std::cout << std::endl;

    // processing
    getRequest();
    processRequest();
    setResponse();

    // checking the process
    std::cout << "============== Server Response ==============" << std::endl;
    std::cout << "Filename: " << this->_filename << std::endl;
    std::cout << "Body length: " << this->_bodyLength << " bytes" << std::endl;
    std::cout << std::endl;
}

DataHolder::~DataHolder(void) {

}

void DataHolder::parsing(std::vector<char> &request) {
    // show the request
    // std::cout << request.data() << std::endl;

    // getline
    std::stringstream ss(request.data());

    // get method
    std::getline(ss, this->_method, ' ');

    // get resource
    std::getline(ss, this->_resource, ' ');

    // get version
    std::getline(ss, this->_cversion, '\n');

}

void DataHolder::getRequest(void) {
    // set content-type
    if (this->_resource.compare("/") == 0) {
        this->_filename = "sites/html/index.html";
        this->_contentType = "text/html";
        return ;
    }

    if (this->_resource.compare("/panda.jpg") == 0) {
        this->_filename = "sites/image/panda.jpeg";
        this->_contentType = "image/jpeg";
        return ;
    }

    // in case cannot find any one of them
    this->_filename = "sites/html/404.html";
    this->_contentType = "text/html";
}

void DataHolder::processRequest(void) {
    // get file size for content-length
    struct stat sb;

    stat(this->_filename.c_str(), &sb);
    this->_bodyLength = sb.st_size;
}

void DataHolder::setResponse(void) {

    // set status line
    this->_sversion = "HTTP/1.1";
    this->_statusCode = "200";
    this->_reason = "OK";

    this->_header.append(this->_sversion);
    this->_header.append(" ");
    this->_header.append(this->_statusCode);
    this->_header.append(" ");
    this->_header.append(this->_reason);
    this->_header.append("\r\n");

    // set header -> Content-type
    this->_header.append("Content-type: ");
    this->_header.append(this->_contentType);
    this->_header.append("\r\n");

    // set header -> Content-length
    this->_header.append("Content-Length: ");
    this->_header.append(this->_contentLength);
    this->_header.append("\r\n\r\n");

    this->_headerLength = this->_header.length();

}