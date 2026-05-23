#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "Server.hpp"
#include "CommandParser.hpp"
#include "Client.hpp"

#include <sstream>

#define CRLF "\r\n"
#define SERVER "lilserv"

enum Numerics {
    // General errors
    ERR_NOSUCHNICK        = 401,
    ERR_NOSUCHCHANNEL     = 403,
    ERR_CANNOTSENDTOCHAN  = 404,
    ERR_TOOMANYCHANNELS   = 405,
    ERR_NORECIPIENT       = 411,
    ERR_NOTEXTTOSEND      = 412,

    // Nick / Channel user errors
    ERR_NONICKNAMEGIVEN   = 431,
    ERR_ERRONEUSNICKNAME  = 432,
    ERR_NICKNAMEINUSE     = 433,
    ERR_NICKCOLLISION     = 436,
    ERR_USERNOTINCHANNEL  = 441,
    ERR_NOTONCHANNEL      = 442,
    ERR_USERONCHANNEL     = 443,

    // Registration / authentication
    ERR_NOTREGISTERED     = 451,
    ERR_NEEDMOREPARAMS    = 461,
    ERR_ALREADYREGISTRED  = 462,
    ERR_PASSWDMISMATCH    = 464,

    // Channel join errors
    ERR_CHANNELISFULL     = 471,
    ERR_UNKNOWNMODE       = 472,
    ERR_INVITEONLYCHAN    = 473,
    ERR_BANNEDFROMCHAN    = 474,
    ERR_BADCHANNELKEY     = 475,

    // Privileges
    ERR_CHANOPRIVSNEEDED  = 482
};

enum RPL {
    RPL_WELCOME  = 1,
    RPL_YOURHOST = 2,
    RPL_CREATED  = 3,
    RPL_MYINFO   = 4
}

class CommandHandler {
    public:
        CommandHandler(Server& server);

        void            handleCommand(s_msg *message, Client& client);
        void            handlePass(s_msg *message, Client& client);
        void            handleNick(s_msg *message, Client& client);
        void            handleUser(s_msg *message, Client& client);
        void            respond(std::string reply, Client& client);
        std::string     formReply(int code, s_msg *message, Client& client);
        void            tryToRegister(Client& client);
        void            welcome(Client& client);
    private:
        Server&                     server;
        std::map<int, std::string>  replies;
};

#endif