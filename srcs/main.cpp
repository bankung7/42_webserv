#include "webserv.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void send_response(int client_socket, const char *response) {
    send(client_socket, response, strlen(response), 0);
}

int main(int argc, char **argv) {

    if (argc > 2) {
        std::cout << "Error: Usage ./webserv [configuration file]" << std::endl;
        return (1);
    }

    if (argc == 2)
        std::cout << "[INFO]: getting configuration file " << argv[1] << std::endl;

    int server_socket,  client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cout << "Socket fail to created" << std::endl;
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        std::cout << "bind fail" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        std::cout << "listen fail" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Server listening on port " << PORT << std::endl;

    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_socket == -1) {
            std::cout << "Client socket fail" << std::endl;
            continue ;
        }
        
        std::cout << "Client connected" << std::endl;

        const char * response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World";

        send_response(client_socket, response);

        close(client_socket);
        std::cout << "Client disconnected" << std::endl;
    }

    close(server_socket);
    return (0);
}