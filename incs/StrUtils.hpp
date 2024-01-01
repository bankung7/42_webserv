#ifndef STRUTILS_HPP
#define STRUTILS_HPP

#include <string>
#include <iostream>
#include <vector>
#include <sstream>

// std::vector<std::string> split(std::string &, char);
std::vector<std::string> split(const std::string& str, const std::string& delimiters);
std::string ltrim(const std::string&, std::string);
std::string rtrim(const std::string&, std::string);
std::string trim(std::string&, std::string);
std::string removeNewlines(const std::string&);
int isvalidport(const std::string&);
int ft_stoi(const std::string&);
int isvalidstatuscode(const std::string&);
size_t ft_stost(const std::string&);

#endif