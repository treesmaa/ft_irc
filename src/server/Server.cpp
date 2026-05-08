#include "Server.hpp"

Server::Server() {}

Server::Server(unsigned int port, char *password) : _port(port), _password(password) {
    std::cout << "Server class created. Port: " << port << ", Password: " << password << std::endl;
    //add checks for password and listening port
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
        throw std::runtime_error(std::string("failed to open socket: ") + strerror(errno));
    //to allow for immediate restart:
    int opt = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error(std::string("failed to set socket options: ") + strerror(errno));
    //to detect dead clients (disconnected without sending a QUIT)
    if (setsockopt(_sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) == -1)
        throw std::runtime_error(std::string("failed to set socket options: ") + strerror(errno));
    //set socket fd in non-blocking mode
    if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error(std::string("failed to set socket to non-blocking: ") + strerror(errno));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);

    if (bind(_sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        throw std::runtime_error(std::string("failed to bind socket: ") + strerror(errno));

    if (listen(_sockfd, SOMAXCONN) == -1)
        throw std::runtime_error(std::string("failed to listen on socket: ") + strerror(errno));

    close(_sockfd);
}