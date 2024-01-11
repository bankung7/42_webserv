#include "StrUtils.hpp"

std::vector<std::string> split(const std::string& str, const std::string& delimiters) {
    std::vector<std::string> tokens;
    std::string::size_type start = 0;
    std::string::size_type end = str.find_first_of(delimiters);

    while (end != std::string::npos) {
        if (end != start) {
            tokens.push_back(str.substr(start, end - start));
        }

        start = end + 1;
        end = str.find_first_of(delimiters, start);
    }

    if (start < str.length()) {
        tokens.push_back(str.substr(start));
    }

    return tokens;
}

//" \t\r\n"
std::string ltrim(const std::string& str, std::string c_list) {
    std::string::size_type start = str.find_first_not_of(c_list);
    return (start == std::string::npos) ? "" : str.substr(start);
}

std::string rtrim(const std::string& str, std::string c_list) {
    std::string::size_type end = str.find_last_not_of(c_list);
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

std::string trim(std::string& str, std::string c_list){
    return (rtrim(ltrim(str, c_list), c_list));
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
    std::istringstream iss(str);
    int result;

    if (!(iss >> result)) {
        // Conversion failed
        std::cerr << "Error: Could not convert string to int." << std::endl;
        return 0; // Or handle the error in an appropriate way for your program
    }

    return result;
}

size_t ft_stost(const std::string& str) {
    std::istringstream iss(str);
    size_t result;

    if (!(iss >> result)) {
        // Conversion failed
        std::cerr << "Error: Could not convert string to size_t." << std::endl;
        return 0; // Or handle the error in an appropriate way for your program
    }

    return result;
}

int isvalidport(const std::string& num) {
    int i_num = ft_stoi(num);
    if (i_num >= 0 && i_num <= 65535)
        return (i_num);
    else
        return (-1);
}

int isvalidstatuscode(const std::string& code) {
    std::string valid_code = "100 101 102 103\
        200 201 202 203 204 205 206 207 208 226\
        300 301 302 303 304 305 306 307 308\
        401 402 403 404 405 406 407 408 409 410 411 412 413 414 415 416 417 418 421 422 423 424 425 426 428 429 431 451\
        500 501 502 503 504 505 506 507 508 510 511";
    if (valid_code.find(code, 3) == std::string::npos)
        return -1;
    return (ft_stoi(code));
}