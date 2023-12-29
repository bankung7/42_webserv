#ifndef CONF_HPP
#define CONF_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "Server.hpp"
#include "StrUtils.hpp"

class Conf {
    private:
        std::string _filename;
        std::string _filedata;
        std::vector< std::vector<std::string> > _result;        
        int _n_server;

    public:
        Conf();
        Conf(std::string);
        ~Conf();
        void readfile(); 
        bool checkbraces();
        bool checkdirectives();
};

#endif