#include "Bot.hpp"

#include <arpa/inet.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

// Constructors & Destructor
Bot::Bot( void ) : _exit(0), _serverfd(-1), _connected(false) {}
Bot::Bot( const Bot& other ) { *this = other; }
Bot::~Bot( void ) {}

// Operators
Bot& Bot::operator=( const Bot& other ) {
    if (this != &other) {
        _exit = other._exit;
        _connected = other._connected;
    }
    return (*this);
}

// General methods
void Bot::connect( const std::string& network, const std::string& port) {
    struct addrinfo hints, *res;

    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(network.c_str(), this->checkPort(port.c_str()), &hints, &res);
    if (status != 0) {
        throw std::runtime_error(gai_strerror(status));
    }

    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {

        // Try opening a socket
        _serverfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (_serverfd == -1) {
            continue;
        }

        // ::connect to not get naming conflict
        if (::connect(_serverfd, p->ai_addr, p->ai_addrlen) == -1) {
            // perror("client: connect");
            close(_serverfd);
            _serverfd = -1;
            continue;
        }

        // set socket fd in non-blocking mode
        if (fcntl(_serverfd, F_SETFL, O_NONBLOCK) == -1) {
            freeaddrinfo(res);
            close(_serverfd);
            _serverfd = -1;
            throw std::runtime_error(std::string("failed to set socket to non-blocking: ") + strerror(errno));
        }
        break;
    }

    freeaddrinfo(res);

    if (p == NULL)
        throw std::runtime_error(std::string("failed to connect: ") + strerror(errno));

    _connected = true;
}

int Bot::run( void ) {
    while (!g_stop && _connected) {
        sleep(1);
        std::cout << "marvin: I am depressed :(" << std::endl;
    }
    return _exit;
}

// Private Member Functions
const char* Bot::checkPort(const char *str) {
    int asInt = std::atoi(str);
    if (asInt < 1024 || asInt > 65535)
        throw std::runtime_error("invalid port number");
    return str;
}
