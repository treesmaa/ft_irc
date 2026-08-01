#ifndef BOT_HPP
#define BOT_HPP

#include <signal.h>
#include <string>
#include <set>

extern volatile sig_atomic_t g_stop; // defined in main.cpp

// RFC 2812: NICK < 10 chars! else 432 ERR_ERRONEUSNICKNAME
#ifndef BOT_NICK
# define BOT_NICK "marvin"
#endif //! BOT_NICK

#ifndef BOT_USER
# define BOT_USER "Zaphod_Beeblebot"
#endif //! BOT_USER

#ifndef IN_PROMPT
# define IN_PROMPT  ">>>  "
#endif //! IN_PROMPT

#ifndef OUT_PROMPT
# define OUT_PROMPT "<<<  "
#endif //!OUT_PROMPT

#define MAX_LENGTH 512
#define CRLF "\r\n"

class Bot {
    public:
        // Constructors & Destructors
        Bot ( void );
        Bot ( const Bot& );
        ~Bot( void );

        // Operators
        Bot& operator=( const Bot& );

        // Public Methods
        void loadBadWords( const char* path );
        void connect     ( const std::string& network, const std::string& port);
        void login       ( const std::string& password );
        void join        ( const std::string& channel );
        int  run         ( void ); // Essiantially the main loop

    private:
        // Private Variables
        int                     _exit;      // to retreive exit status
        int                     _serverfd;
        bool                    _connected;
        std::string             _buf;
        std::set<std::string>   _badWords;

        // Private Member Functions
        void sendToServer  (const std::string& message);
        void readFromServer( void );
        void processBuffer ( void );
        void processMessage( const std::string& message );

        // Bot Commands
        void featGreet   ( const std::string& joiner, const std::string& channel );
        void featAutoJoin( const std::string& channel );
        void featMonitor ( const std::string& reveicer, const std::string& sender,
                           const std::string& token );

        // Server Commands
        void sendPASS   ( const std::string& password );
        void sendNICK   ( const std::string& nickname );
        void sendUSER   ( const std::string& username, const std::string& mode,
                          const std::string& unused,   const std::string& msg );
        void sendJOIN   ( const std::string& channel);
        void sendPRIVMSG( const std::string& receiver, const std::string& in_msg );

        // Helper Functions
        std::string tolower      ( const std::string& token );
        const char* checkPort    (const char *str);
        std::string sanitizeToken( const std::string& sender );
};

#endif // ! BOT_HPP
