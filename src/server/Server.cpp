#include "Server.hpp"

Server::Server() {}

Server::Server(int port, char *password) : _port(port), _password(password), _sockfd(-1) {
    std::cout << "Server class created. Port: " << port << ", Password: " << password << std::endl;
    //add checks for password?
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
    if (_sockfd != -1)
        close(_sockfd);
    std::cout << "Server class destroyed" << std::endl;
}

std::string intToString(int n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

void Server::boot() {
    struct addrinfo hints;
    struct addrinfo *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, intToString(_port).c_str(), &hints, &servinfo);
    if ((status != 0))
        throw std::runtime_error(gai_strerror(status));
    
    struct addrinfo *p;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        _sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (_sockfd == -1)
            continue;
        int opt = 1;
        if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket options: ") + strerror(errno));
        }
        //to detect dead clients (disconnected without sending a QUIT)
        if (setsockopt(_sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket options: ") + strerror(errno));
        }
        //set socket fd in non-blocking mode
        if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket to non-blocking: ") + strerror(errno));
        }
        if (bind(_sockfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        
        close(_sockfd);
        _sockfd = -1;
    }

    freeaddrinfo(servinfo);

    if (p == NULL)
        throw std::runtime_error(std::string("failed to bind socket: ") + strerror(errno));

    if (listen(_sockfd, SOMAXCONN) == -1)
        throw std::runtime_error(std::string("failed to listen on socket: ") + strerror(errno));

    std::cout << "IRC server listening on port " << _port << std::endl;
    while (1) {
        // 1. accept new clients (non-blocking socket helps)
        // 2. read from existing clients
        // 3. handle disconnects
    }
}