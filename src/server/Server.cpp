#include "Server.hpp"

Server::Server() {}

Server::Server(char *port, char *password) : _port(port), _password(password) {
    std::cout << "Server class created. Port: " << port << ", Password: " << password << std::endl;
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