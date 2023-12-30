#include "Webserv.hpp"
#include "Conf.hpp"


int main(int ac, char **av)
{
    if (ac == 1)
        Webserv sv;
    else if (ac == 2)
        Webserv sv(av[1]);
    else
        std::cout << "Usage: ./webserv [configuration file]" << std::endl;
    return (0);
}