#include "Bot.hpp"
#include <iostream>
#include <sstream>

// Constructors & Destructor
Bot::Bot( void ) : _exit(0), _connected(false) {}
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
int Bot::connect( const std::string& network, const std::string& port, const std::string& password ) {
    (void)network;
    (void)port;
    (void)password;
    return 0;
}

int Bot::run( void ) {
    while (!g_stop && _connected) {
        sleep(1);
        std::cout << "marvin: I am depressed :(" << std::endl;
    }
    return _exit;
}

// Private Member Functions
int Bot::convertPort(char *str) {
    std::stringstream ss(str);
    int port;

    ss >> port;
    if (ss.fail() || !ss.eof() || port < 1024 || port > 65535)
        throw std::runtime_error("invalid port number");
    return port;
}
