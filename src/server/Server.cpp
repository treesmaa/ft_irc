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
        int yes = 1;
        if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket options: ") + strerror(errno));
        }
        //to detect dead clients (disconnected without sending a QUIT)
        if (setsockopt(_sockfd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket options: ") + strerror(errno));
        }
        //set socket fd in non-blocking mode
/*         if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket to non-blocking: ") + strerror(errno));
        } */
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

        //poll() or similar for non-blocking
        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int new_fd = accept(_sockfd, (struct sockaddr *)&client_addr, &addr_size);//i guess we need a new fd for each connecting client
        if (new_fd == -1)
            throw std::runtime_error(std::string("failed to accept connection: ") + strerror(errno));
        //for IPv4 and IPv6 get their address in printable presentation with inet_ntop()
        char ipstr[INET6_ADDRSTRLEN];
        const char *printable_addr;
        if (client_addr.ss_family == AF_INET) {
            struct sockaddr_in *s = (struct sockaddr_in *)&client_addr;
            printable_addr = inet_ntop(client_addr.ss_family, &s->sin_addr, ipstr, sizeof(ipstr));
        }
        else {
            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&client_addr;
            printable_addr = inet_ntop(client_addr.ss_family, &s->sin6_addr, ipstr, sizeof(ipstr));
        }
        if (!printable_addr)
            throw std::runtime_error(std::string("inet_ntop() failed") + strerror(errno));

        std::cout << "Connection from " << ipstr << std::endl;
        // 1. accept new clients (non-blocking socket helps)
        // 2. read from existing clients
        // 3. handle disconnects
    }
}