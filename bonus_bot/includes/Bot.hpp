#ifndef BOT_HPP
#define BOT_HPP

#include <string>

#ifndef BOT_NICK
# define BOT_NICK "kombot"
#endif // ! BOT_NICK

#ifndef BOT_USER
# define BOT_USER "Zaphod Beeblebot"
#endif // ! BOT_USER

class Bot {
    public:
        // Constructors & Destructors
        Bot( void );
        Bot( const Bot& );
        ~Bot( void );

        // General Methods
        bool connect( const std::string& network, const int& port, const std::string& password ); // TODO: Add optional channel
        // Operators
        Bot& operator=( const Bot& );

        // Getters
        int exit( void );

        // Setters
    private:
        int _exit; // to retreive exit status
};

#endif // ! BOT_HPP
