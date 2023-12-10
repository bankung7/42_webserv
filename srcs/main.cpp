#include "Webserv.hpp"

int main(int argc, char **argv) {

    // TODO: check argument and the incoming config file
    (void)argc, (void)argv;

    // TODO : Server part
    Webserv wsv;

    if (argc == 2)
        wsv.set_config_name(std::string(argv[1]));
    else
        wsv.set_config_name(std::string("/config/default.conf"));

    try {
        wsv.start();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return (0);
}