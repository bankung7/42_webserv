#include "StrUtils.hpp"

std::vector<std::string> split(std::string &str, char delimiter) {
    std::vector<std::string> tokens;
    std::string::size_type start = 0;
    std::string::size_type end = str.find(delimiter);
    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

std::string ltrim(const std::string& str) {
    std::string::size_type start = str.find_first_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : str.substr(start);
}

std::string rtrim(const std::string& str) {
    std::string::size_type end = str.find_last_not_of(" \t\r\n");
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

// Function to remove all newline characters from a string
std::string removeNewlines(const std::string& str) {
    std::string result = str;

    for (std::string::iterator it = result.begin(); it != result.end(); ++it) {
        if (*it == '\n' || *it == '\r') {
            it = result.erase(it);
            --it;
        }
    }

    return result;
}

int ft_stoi(const std::string& str) {
    int result = 0;
    bool negative = false;
    std::size_t i = 0;
    if (str[i] == '-') {
        negative = true;
        ++i;
    } else if (str[i] == '+') {
        ++i;
    }
    while (i < str.length() && isdigit(str[i])) {
        result = result * 10 + (str[i] - '0');
        ++i;
    }
    return (negative) ? -result : result;
}

int isvalidport(const std::string& num) {
    int i_num = ft_stoi(num);
    if (i_num >= 0 && i_num <= 65535)
        return (i_num);
    else
        return (-1);
}
