#ifndef STRUTILS_HPP
#define STRUTILS_HPP

#include <string>
#include <vector>

std::vector<std::string> split(std::string &, char);
std::string ltrim(const std::string& str);
std::string rtrim(const std::string& str);
std::string removeNewlines(const std::string& str);
int isvalidport(const std::string&);
int ft_stoi(const std::string&);

#endif