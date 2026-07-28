#include "Bot.hpp"

Bot::Bot( void ) : _exit(0) {}
Bot::Bot( const std::string& network, const int& port, const std::string& password ) : _exit(0) {
    (void)network;
    (void)port;
    (void)password;
}

Bot::Bot( const Bot& other ) { *this = other; }
Bot::~Bot( void ) {}

// Getters
int Bot::exit( void ) { return _exit; }

// Operators
Bot& Bot::operator=( const Bot& other ) {
    if (this != &other) {
        _exit = other._exit;
    }
    return (*this);
}
