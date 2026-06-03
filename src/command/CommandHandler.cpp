#include "CommandHandler.hpp"
#include "Server.hpp"
//important: '%s' should be removed from the reply string!
//it's just to show which replies take additional parameters.
std::map<int, std::string> initReplies() {
    std::map<int, std::string> r;

    // Connection / Registration
    r[ERR_NOTREGISTERED]      = " :You have not registered";                  // 451
    r[ERR_NEEDMOREPARAMS]     = " :Not enough parameters";                 // 461
    r[ERR_ALREADYREGISTRED]   = " :You may not reregister";                   // 462
    r[ERR_PASSWDMISMATCH]     = " :Password incorrect";                       // 464

    // Nickname
    r[ERR_NONICKNAMEGIVEN]    = " :No nickname given";                        // 431
    r[ERR_ERRONEUSNICKNAME]   = " :Erroneous nickname";                    // 432
    r[ERR_NICKNAMEINUSE]      = " :Nickname is already in use";            // 433

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
    r[ERR_NORECIPIENT]        = " :No recipient given (%s)";                  // 411
    r[ERR_NOTEXTTOSEND]       = " :No text to send";                          // 412
    r[ERR_CANNOTSENDTOCHAN]   = "%s :Cannot send to channel";                // 404
    r[ERR_NOSUCHNICK]         = "%s :No such nick/channel";                  // 401

    // MODE
    r[ERR_UNKNOWNMODE]        = "%c :is unknown mode char to me";            // 472

    // INVITE
    r[ERR_USERNOTINCHANNEL]   = "%s %s :They aren't on that channel";        // 441
    r[ERR_NOTONCHANNEL]       = "%s :You're not on that channel";            // 442

    return r;
}

CommandHandler::CommandHandler(Server& server) : _server(server) {
    _replies = initReplies();
}

std::string CommandHandler::formReply(int code, s_msg *message, Client& client) {
    std::ostringstream oss;
    if (code == ERR_NEEDMOREPARAMS)
        oss << ":" << SERVER << " " << code << " " << client.getNickname() << " " << message->command << _replies[code] << CRLF;
    else if (code == ERR_ERRONEUSNICKNAME || code == ERR_NICKNAMEINUSE)
        oss << ":" << SERVER << " " << code << " " << client.getNickname() << " " << message->parameters[0] << _replies[code] << CRLF;
    else
        oss << ":" << SERVER << " " << code << " " << client.getNickname() << _replies[code] << CRLF;
    return oss.str();
}

void CommandHandler::respond(std::string reply, Client& client) {
    if (send(client.getFd(), reply.c_str(), reply.size(), 0) == -1)
        std::cerr << "Send error" << std::endl;
}

void CommandHandler::welcome(Client& client) {
    std::string nick = client.getNickname();
    std::string hostmask = nick + "!" + client.getUsername() + "@" + client.getHost();

    std::string rpl_welcome = ":" + std::string(SERVER) + RPL_WELCOME + nick + " :Welcome to the Internet Relay Network " + hostmask + CRLF;
    std::string rpl_yourhost = ":" + std::string(SERVER) + RPL_YOURHOST + nick + " :Your host is " + std::string(SERVER) + ", running version 1.0" + CRLF;
    std::string rpl_created = ":" + std::string(SERVER) + RPL_CREATED + nick + " :This server was created " + _server.getCreationDate() + CRLF;
    std::string rpl_myinfo = ":" + std::string(SERVER) + RPL_MYINFO + nick + " " + std::string(SERVER) + " 1.0 io itkol" + CRLF;
    
    if (send(client.getFd(), rpl_welcome.c_str(), rpl_welcome.size(), 0) == -1)
        std::cerr << "Send error" << std::endl;
    if (send(client.getFd(), rpl_yourhost.c_str(), rpl_yourhost.size(), 0) == -1)
        std::cerr << "Send error" << std::endl;
    if (send(client.getFd(), rpl_created.c_str(), rpl_created.size(), 0) == -1)
        std::cerr << "Send error" << std::endl;
    if (send(client.getFd(), rpl_myinfo.c_str(), rpl_myinfo.size(), 0) == -1)
        std::cerr << "Send error" << std::endl;
}

void CommandHandler::tryToRegister(Client& client) {
    if (client.getNickname() == "*" || client.getUsername().empty())
        return;
    std::string server_pw = _server.getPassword();
    if (!server_pw.empty()) {
        if (client.getPassword() != server_pw) {
            respond(formReply(ERR_PASSWDMISMATCH, NULL, client), client);
            return;
        }
        client.registerClient();
        welcome(client);
    }
}

void CommandHandler::handlePass(s_msg *message, Client& client) {
    if (client.isRegistered()) {
        respond(formReply(ERR_ALREADYREGISTRED, message, client), client);
        return;
    }
    if (message->parameters.empty()) {
        respond(formReply(ERR_NEEDMOREPARAMS, message, client), client);
        return;
    }
    client.setPassword(message->parameters[0]);
    tryToRegister(client);
}

bool isSpecial(char c) {
    return ((c >= 0x5B && c <= 0x60) || (c >= 0x7B && c <= 0x7D));
}

bool isValidNickname(std::string& nick) {
    if (nick.empty() || nick.length() > 9)
        return false;
    if (!isalpha(nick[0]) && !isSpecial(nick[0]))
        return false;
    for (size_t i = 1; i < nick.size(); i++) {
        if (!isalnum(nick[i]) && !isSpecial(nick[i]) && nick[i] != '-')
            return false;
    }
    return true;
}

bool nickInUse(std::string& nick, std::map<int, Client> clients) {
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.getNickname() == nick) {
            return true;
        }
    }
    return false;
}

void CommandHandler::handleNick(s_msg *message, Client& client) {
    if (message->parameters.empty()) {
        respond(formReply(ERR_NONICKNAMEGIVEN, message, client), client);
        return;
    }
    if (!isValidNickname(message->parameters[0])){
        respond(formReply(ERR_ERRONEUSNICKNAME, message, client), client);
        return;
    }
    if (nickInUse(message->parameters[0], _server.getClients())) {
        respond(formReply(ERR_NICKNAMEINUSE, message, client), client);
        return;
    }
    client.setNickname(message->parameters[0]);
    tryToRegister(client);
}

void CommandHandler::handleUser(s_msg *message, Client& client) {
    if (client.isRegistered()) {
        respond(formReply(ERR_ALREADYREGISTRED, message, client), client);
        return;
    }
    else if (message->parameters.size() < 4) {
        respond(formReply(ERR_NEEDMOREPARAMS, message, client), client);
        return;
    }
    client.setUsername(message->parameters[0]);
    tryToRegister(client);
}

void CommandHandler::handleQuit(s_msg* message, Client& client) {
    //custom message handling
    //send message to clients who are in the same channels as the client
    std::string reason;
    if (!message->parameters.empty())
        reason = "quit:" + message->parameters[0];
    else
        reason = "quit";
    _server.disconnectClient(client, reason);
}

void CommandHandler::handleCommand(s_msg *message, Client& client) {
    if (message->command == "PASS")
        handlePass(message, client);
    else if (message->command == "NICK")
        handleNick(message, client);
    else if (message->command == "USER")
        handleUser(message, client);
    else if (message->command == "QUIT")
        handleQuit(message, client);
}

