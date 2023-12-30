#ifndef CONF_HPP
#define CONF_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "Server.hpp"
#include "Webserv.hpp"
#include "StrUtils.hpp"

class Conf {
    private:
        std::string _filename;
        std::string _filedata;
        int _n_server;

    public:
        Conf();
        Conf(std::string);
        ~Conf();
        void readfile(); 
        bool checkconf();
        
        // Check each directive
        bool checkroot(std::vector<std::string>);
        bool checkmaxclientsize(std::vector<std::string>);
        bool checkport(std::vector<std::string>);
        bool checkservername(std::vector<std::string>);
        bool checkemptyline(std::vector<std::string>);
        bool checkallowedmethods(std::vector<std::string>);
        bool checkerrorpage(std::vector<std::string>);
        bool checkclosebraces(std::vector<std::string>);
        bool checkreturn(std::vector<std::string>);
        bool checkallowfileupload(std::vector<std::string>);
        bool checkautoindex(std::vector<std::string>);
        bool checkserver(std::vector<std::string>);
        bool checklocation(std::vector<std::string>);
        bool checkindex(std::vector<std::string>);

};

#endif