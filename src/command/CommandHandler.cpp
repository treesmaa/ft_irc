#include "CommandHandler.hpp"

std::map<int, std::string> initReplies() {
    std::map<int, std::string> r;

    // Connection / Registration
    r[ERR_NOTREGISTERED]      = ":You have not registered";                  // 451
    r[ERR_NEEDMOREPARAMS]     = "%s :Not enough parameters";                 // 461
    r[ERR_ALREADYREGISTRED]   = ":You may not reregister";                   // 462
    r[ERR_PASSWDMISMATCH]     = ":Password incorrect";                       // 464

    // Nickname
    r[ERR_NONICKNAMEGIVEN]    = ":No nickname given";                        // 431
    r[ERR_ERRONEUSNICKNAME]   = "%s :Erroneous nickname";                    // 432
    r[ERR_NICKNAMEINUSE]      = "%s :Nickname is already in use";            // 433
    r[ERR_NICKCOLLISION]      = "%s :Nickname collision KILL";               // 436

    // Channel / JOIN
    r[ERR_NOSUCHCHANNEL]      = "%s :No such channel";                       // 403
    r[ERR_TOOMANYCHANNELS]    = "%s :You have joined too many channels";     // 405
    r[ERR_CHANNELISFULL]      = "%s :Cannot join channel (+l)";              // 471
    r[ERR_INVITEONLYCHAN]     = "%s :Cannot join channel (+i)";              // 473
    r[ERR_BANNEDFROMCHAN]     = "%s :Cannot join channel (+b)";              // 474
    r[ERR_BADCHANNELKEY]      = "%s :Cannot join channel (+k)";              // 475
    r[ERR_USERONCHANNEL]      = "%s %s :is already on channel";              // 443
    r[ERR_NOTONCHANNEL]       = "%s :You're not on that channel";            // 442
    r[ERR_USERNOTINCHANNEL]   = "%s %s :They aren't on that channel";        // 441

    // Channel operator permissions
    r[ERR_CHANOPRIVSNEEDED]   = "%s :You're not channel operator";           // 482

    // Messaging
    r[ERR_NORECIPIENT]        = ":No recipient given (%s)";                  // 411
    r[ERR_NOTEXTTOSEND]       = ":No text to send";                          // 412
    r[ERR_CANNOTSENDTOCHAN]   = "%s :Cannot send to channel";                // 404
    r[ERR_NOSUCHNICK]         = "%s :No such nick/channel";                  // 401

    // MODE
    r[ERR_UNKNOWNMODE]        = "%c :is unknown mode char to me";            // 472

    // INVITE
    r[ERR_USERNOTINCHANNEL]   = "%s %s :They aren't on that channel";        // 441
    r[ERR_NOTONCHANNEL]       = "%s :You're not on that channel";            // 442

    return r;
}

CommandHandler::CommandHandler(Server& server) : server(server) {
    replies = initReplies();
}

char *CommandHandler::formReply(int code, Client& client) {
    //continue
}

void CommandHandler::respond(int code, Client& client) {
    char *reply = formReply(code, client);
    if (send(client.getFd(), replies[code].c_str(), replies[code].size(), 0) == -1)
        std::cerr << "Send error" << std::endl;
}

void CommandHandler::handlePass(s_msg *message, Client& client) {
    if (message->parameters.size() == 0) {
        respond(ERR_NEEDMOREPARAMS, client);
        return;
    }
    if (message->parameters[0] == client.getServer()->getPassword())
        client.authenticate();
    else
        respond(ERR_PASSWDMISMATCH, client);
}

/* int handleNick(s_msg *message, Client& client) {
    if (!client.isAuthenticated() == true) {
        
    }
}
 */

void CommandHandler::handleCommand(s_msg *message, Client& client) {
    if (message->command == "PASS")
        handlePass(message, client);
/*     else if (message->command == "NICK")
        handleNick(message, client); */
    //else if (message->command == "USER")


}

