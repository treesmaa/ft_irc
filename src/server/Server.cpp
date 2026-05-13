#include "Server.hpp"

Server::Server() {}

Server::Server(int port, char *password) : port(port), password(password), server_fd(-1) {
    std::cout << "Server class created. Port: " << port << ", Password: " << password << std::endl;
    //add checks for password?
}

Server::~Server() {
    if (server_fd != -1)
        close(server_fd);
    std::cout << "Server class destroyed" << std::endl;
}

std::string intToString(int n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

void Server::addToPoll(int fd) {
        struct pollfd p;
        p.fd = fd;
        p.events = POLLIN;
        p.revents = 0;
        pfds.push_back(p);
}

void Server::printNewClient(struct sockaddr_storage client_addr) const {
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
}

void Server::serverSocketSetup() {
    struct addrinfo hints;
    struct addrinfo *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, intToString(port).c_str(), &hints, &servinfo);
    if ((status != 0))
        throw std::runtime_error(gai_strerror(status));
    
    struct addrinfo *p;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server_fd == -1)
            continue;
        int yes = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket options: ") + strerror(errno));
        }
        //to detect dead clients (disconnected without sending a QUIT)
        if (setsockopt(server_fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket options: ") + strerror(errno));
        }
        //set socket fd in non-blocking mode
/*         if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket to non-blocking: ") + strerror(errno));
        } */
        if (bind(server_fd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        
        close(server_fd);
        server_fd = -1;
    }

    freeaddrinfo(servinfo);

    if (p == NULL)
        throw std::runtime_error(std::string("failed to bind socket: ") + strerror(errno));

    if (listen(server_fd, SOMAXCONN) == -1)
        throw std::runtime_error(std::string("failed to listen on socket: ") + strerror(errno));

    addToPoll(server_fd);
    std::cout << "IRC server listening on port " << port << std::endl;
}

void Server::boot() {

    serverSocketSetup();

    while (1) {

        if (poll(&pfds[0], pfds.size(), -1) == -1)
            throw std::runtime_error(std::string("poll() failed: ") + strerror(errno));

        for (int i = 0; i < pfds.size(); i++) {//loop through all pfds
            if (pfds[i].revents & POLLIN) {
                if (pfds[i].fd == server_fd) {

                }
            }
            else {

            }
        }
        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);//i guess we need a new fd for each connecting client
        if (new_fd == -1)
            throw std::runtime_error(std::string("failed to accept connection: ") + strerror(errno));
        addToPoll(new_fd, POLLIN);
        printNewClient(client_addr);

        // 1. accept new clients (non-blocking socket helps)
        // 2. read from existing clients
        // 3. handle disconnects
    }
}