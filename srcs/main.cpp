#include "Server.hpp"

int main(int argc, char **argv) {

    // check argument and the incoming config file
    (void)argc, (void)argv;

    // TODO : Server part
    Server sv;

    if (argc == 2)
        sv.set_config_name(std::string(argv[1]));
    else
        sv.set_config_name(std::string("/config/default.conf"));

    sv.start();

    return (0);
}