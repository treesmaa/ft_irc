#include "Server.hpp"

Server::Server() {}

Server::Server(char *port, char *password) : _port(port), _password(password), _addr{} {
    std::cout << "Server class created. Port: " << port << ", Password: " << password << std::endl;
    //add checks for password and listening port
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(atoi(_port));
}

Server::Server(const Server& original) {
    _port = original._port;
    _password = original._password;
}
Server& Server::operator=(const Server& other) {
    if (this != &other) {
        _port = other._port;
        _password = other._password;
    }
    return *this;
}

Server::~Server() {
    std::cout << "Server class destroyed" << std::endl;
}

void Server::boot() {
    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_sockfd == -1)
        throw std::runtime_error(std::string("Error: failed to open socket: ") + strerror(errno));

    if (bind(_sockfd, (struct sockaddr*)&_addr, sizeof(_addr)) == -1)
        throw std::runtime_error(std::string("Error: failed to bind socket: ") + strerror(errno));
    //make sure you understand really sockets, bind, address etc.
    //listen...

    close(_sockfd);
}