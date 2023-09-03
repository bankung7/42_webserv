#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// variable
#define PORT 8080
#define BUFFER_SIZE 1024

// c++ library
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <map>
#include <vector>

// c library
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>

// class
#include "Server.hpp"
#include "DataHolder.hpp"

#endif
