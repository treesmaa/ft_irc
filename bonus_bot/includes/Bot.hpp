#ifndef BOT_HPP
#define BOT_HPP

#include <signal.h>

extern volatile sig_atomic_t g_stop; // defined in main.cpp

#include <string>

// RFC 2812: NICK < 10 chars! else 432 ERR_ERRONEUSNICKNAME
#ifndef BOT_NICK
# define BOT_NICK "marvin"
#endif // ! BOT_NICK

#ifndef BOT_USER
# define BOT_USER "Zaphod Beeblebot"
#endif // ! BOT_USER

class Bot {
    public:
        // Constructors & Destructors
        Bot( void );
        Bot( const std::string& network, const int& port, const std::string& password );
        Bot( const Bot& );
        ~Bot( void );

        // Operators
        Bot& operator=( const Bot& );

        // General Methods
        int run(); // Essiantially the main loop

        // Getters
        int exit( void );

        // Setters
    private:
        // Private Variables
        int _exit; // to retreive exit status
};

#endif // ! BOT_HPP
