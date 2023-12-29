#include "Webserv.hpp"
#include "Conf.hpp"


int main(int ac, char **av)
{
    if (ac == 1)
    {
        Conf cf;
        // Webserv sv;
    }
    else if (ac == 2)
    {
        Conf cf(av[1]);
        // Webserv sv;
    }
    else
        std::cout << "Usage: ./webserv [configuration file]" << std::endl;
    return (0);
}