#ifndef BOT_HPP
#define BOT_HPP

#include <signal.h>
#include <string>

extern volatile sig_atomic_t g_stop; // defined in main.cpp

// RFC 2812: NICK < 10 chars! else 432 ERR_ERRONEUSNICKNAME
#ifndef BOT_NICK
# define BOT_NICK "marvin"
#endif // ! BOT_NICK

#ifndef BOT_USER
# define BOT_USER "Zaphod_Beeblebot"
#endif // ! BOT_USER

#ifndef MAX_LENGTH
# define MAX_LENGTH 512
#endif // ! MAX_LENGTH

#define CRLF "\r\n"

class Bot {
    public:
        // Constructors & Destructors
        Bot( void );
        Bot( const Bot& );
        ~Bot( void );

        // Operators
        Bot& operator=( const Bot& );

        // General Methods
        void connect( const std::string& network, const std::string& port);
        void login( const std::string& password );
        int run(); // Essiantially the main loop

        // Getters
        int exit( void );

        // Setters
    private:
        // Private Variables
        int         _exit;      // to retreive exit status
        int         _serverfd;
        bool        _connected;
        std::string _buf;

        // Private Member Functions
        const char* checkPort(const char *str);
        void sendToServer(const std::string& message);
        void readFromServer( void );
};

#endif // ! BOT_HPP
