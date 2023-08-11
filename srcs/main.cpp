#include "webserv.hpp"

int main(int argc, char **argv) {

    if (argc > 2) {
        std::cout << "Error: Usage ./webserv [configuration file]" << std::endl;
        return (1);
    }

    if (argc == 2)
        std::cout << "[INFO]: getting configuration file " << argv[1] << std::endl;

    return (0);
}