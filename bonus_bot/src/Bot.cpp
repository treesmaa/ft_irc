#include "Bot.hpp"
#include <iostream>

// Constructors & Destructor
Bot::Bot( void ) : _exit(0) {}
Bot::Bot( const std::string& network, const int& port, const std::string& password ) : _exit(0) {
    (void)network;
    (void)port;
    (void)password;
}
Bot::Bot( const Bot& other ) { *this = other; }
Bot::~Bot( void ) {}

// Operators
Bot& Bot::operator=( const Bot& other ) {
    if (this != &other) {
        _exit = other._exit;
    }
    return (*this);
}

int Bot::run( void ) {
    while (!g_stop) {
        sleep(1);
        std::cout << "marvin: I am depressed :(" << std::endl;
    }
    return _exit;
}
