#ifndef BOT_HPP
#define BOT_HPP

#include <signal.h>
#include <string>
#include <set>
#include <vector>

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
        Bot ( const char* execName );
        Bot ( const Bot& );
        ~Bot( void );

        // Operators
        Bot& operator=( const Bot& );

        // Public Methods
        void loadBadWords( const char* path );
        void loadJokes   ( const char* path );
        void connect     ( const std::string& network, const std::string& port);
        void login       ( const std::string& password );
        void join        ( const std::string& channel );
        int  run         ( void ); // Essiantially the main loop

    private:
        // Private Variables
        int                      _exit;      // to retreive exit status
        int                      _serverfd;
        bool                     _connected;
        std::string              _buf;
        std::set<std::string>    _badWords;
        std::vector<std::string> _jokes;
        std::string              _execName;

        // Private Member Functions
        void sendToServer  (const std::string& message);
        void readFromServer( void );
        void processBuffer ( void );
        void processMessage( const std::string& message );

        // Bot Commands
        void featGetTime    ( const std::string& receiver );
        void featGreet      ( const std::string& joiner, const std::string& channel );
        void featAutoJoin   ( const std::string& channel );
        void featMonitor    ( const std::string& reveicer, const std::string& sender,
                              const std::string& token );
        void featJoke       ( const std::string& receiver );
        /*
        *  @param range - A colon seperated range of "min:max"
        */
        void featRandNb     ( const std::string& receiver, const std::string& range );

        // Server Commands
        void sendPASS   ( const std::string& password );
        void sendNICK   ( const std::string& nickname );
        void sendUSER   ( const std::string& username, const std::string& mode,
                          const std::string& unused,   const std::string& msg );
        void sendJOIN   ( const std::string& channel);
        void sendPRIVMSG( const std::string& receiver, const std::string& in_msg );
        void sendPONG   ( const std::string& token );

        // Helper Functions
        std::string tolower       ( const std::string& token );
        const char* checkPort     (const char *str);
        std::string sanitizeToken ( const std::string& sender );
        int         randomNb      ( const size_t min, const size_t max );
        bool        isNumeric     ( const std::string& tok );
        std::string getCurrentTime( void );
};

#endif // ! BOT_HPP
