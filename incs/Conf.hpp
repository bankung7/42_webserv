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
        std::vector<std::string> _serverconf;
        size_t _n_server;
        bool _root_flag;

    public:
        Conf();
        Conf(std::string);
        ~Conf();
        void readfile(); 
        bool checkconf();
        void parseconf(std::vector<Server> &);

        // Getter
        size_t get_n_server();
        std::string get_serverconf(size_t);
        
        // Check Directives
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
        bool checkerrorpageinlocation(std::vector<std::string>);
        bool checkuploadpath(std::vector<std::string>);

};

#endif