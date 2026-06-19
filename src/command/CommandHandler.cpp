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
    r[ERR_NOSUCHCHANNEL]      = " :No such channel";                       // 403
    r[ERR_TOOMANYCHANNELS]    = " :You have joined too many channels";     // 405
    r[ERR_CHANNELISFULL]      = " :Cannot join channel (+l)";              // 471
    r[ERR_INVITEONLYCHAN]     = " :Cannot join channel (+i)";              // 473
    r[ERR_BANNEDFROMCHAN]     = " :Cannot join channel (+b)";              // 474
    r[ERR_BADCHANNELKEY]      = " :Cannot join channel (+k)";              // 475
    r[ERR_USERONCHANNEL]      = " :is already on channel";              // 443
    r[ERR_NOTONCHANNEL]       = " :You're not on that channel";            // 442
    r[ERR_USERNOTINCHANNEL]   = " :They aren't on that channel";        // 441

    // Channel operator permissions
    r[ERR_CHANOPRIVSNEEDED]   = " :You're not channel operator";           // 482

    // Messaging
    r[ERR_NORECIPIENT]        = " :No recipient given";                  // 411
    r[ERR_NOTEXTTOSEND]       = " :No text to send";                          // 412
    r[ERR_CANNOTSENDTOCHAN]   = " :Cannot send to channel";                // 404
    r[ERR_NOSUCHNICK]         = " :No such nick/channel";                  // 401

    // MODE
    r[ERR_UNKNOWNMODE]        = "%c :is unknown mode char to me";            // 472

    // INVITE
    r[ERR_NOTONCHANNEL]       = " :You're not on that channel";            // 442

    return r;
}

CommandHandler::CommandHandler(Server& server) : _server(server) {
    _replies = initReplies();
}

//forms a numeric response for those responses where an extra string (eg command, nickname, channel name) is needed
//modify as needed!
std::string CommandHandler::formReply(int code, std::string str, Client& client) {
    std::ostringstream oss;
    if (code == ERR_NEEDMOREPARAMS || code == ERR_ERRONEUSNICKNAME || code == ERR_NICKNAMEINUSE
        || code == ERR_NOSUCHNICK || code == ERR_CANNOTSENDTOCHAN || code == ERR_NOSUCHCHANNEL || code == ERR_TOOMANYCHANNELS 
        || code == ERR_CHANNELISFULL || code == ERR_INVITEONLYCHAN || code == ERR_BANNEDFROMCHAN || code == ERR_BADCHANNELKEY
        || code == ERR_NOTONCHANNEL || code == ERR_CHANOPRIVSNEEDED || code == ERR_NOTONCHANNEL)
        oss << ":" << SERVER << " " << code << " " << client.getNickname() << " " << str << _replies[code] << CRLF;
    else if (code == ERR_NORECIPIENT)
        oss << ":" << SERVER << " " << code << " " << client.getNickname() << _replies[code] << " (" << str << ")" << CRLF;
    else
        oss << ":" << SERVER << " " << code << " " << client.getNickname() << _replies[code] << CRLF;
    return oss.str();
}
//forms a numeric response that takes no extra string
std::string CommandHandler::formReply(int code, Client& client) {
    std::ostringstream oss;
    oss << ":" << SERVER << " " << code << " " << client.getNickname() << _replies[code] << CRLF;
    return oss.str();
}

std::string CommandHandler::formReply(int code, std::string nick, std::string command, Client& client) {
    std::ostringstream oss;
    if (code == ERR_USERONCHANNEL || code == ERR_USERNOTINCHANNEL)
        oss << ":" << SERVER << " " << code << " " << client.getNickname() << nick << " " << command << _replies[code] << CRLF;
    else
        oss << ":" << SERVER << " " << code << " " << client.getNickname() << _replies[code] << CRLF;
    return oss.str();
}

void CommandHandler::welcome(Client& client) {
    std::string nick = client.getNickname();
    std::string hostmask = nick + "!" + client.getUsername() + "@" + client.getHost();

    std::string rpl_welcome = ":" + std::string(SERVER) + RPL_WELCOME + nick + " :Welcome to the Internet Relay Network " + hostmask + CRLF;
    std::string rpl_yourhost = ":" + std::string(SERVER) + RPL_YOURHOST + nick + " :Your host is " + std::string(SERVER) + ", running version 1.0" + CRLF;
    std::string rpl_created = ":" + std::string(SERVER) + RPL_CREATED + nick + " :This server was created " + _server.getCreationDate() + CRLF;
    std::string rpl_myinfo = ":" + std::string(SERVER) + RPL_MYINFO + nick + " " + std::string(SERVER) + " 1.0 io itkol" + CRLF;
    
    _server.sendToClient(client.getFd(), rpl_welcome.c_str());
    _server.sendToClient(client.getFd(), rpl_yourhost.c_str());
    _server.sendToClient(client.getFd(), rpl_created.c_str());
    _server.sendToClient(client.getFd(), rpl_myinfo.c_str());
}

void CommandHandler::tryToRegister(Client& client) {
    if (client.isRegistered())
        return;
    if (client.getNickname() == "*" || client.getUsername().empty())
        return;
    std::string server_pw = _server.getPassword();
    if (!server_pw.empty()) {
        if (client.getPassword() != server_pw) {
            _server.sendToClient(client.getFd(), formReply(ERR_PASSWDMISMATCH, client));
            return;
        }
    }
    client.registerClient();
    welcome(client);
}

void CommandHandler::handlePass(s_msg *message, Client& client) {
    if (client.isRegistered()) {
        _server.sendToClient(client.getFd(), formReply(ERR_ALREADYREGISTRED, client));
        return;
    }
    if (message->parameters.empty()) {
        _server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, client));
        return;
    }
    client.setPassword(message->parameters[0]);
    tryToRegister(client);
}

std::set<int> getRecipients(std::set<std::string>& client_channels, std::map<std::string, Channel>& all_channels) {
    std::set<int> recipients;
    for (std::set<std::string>::iterator it = client_channels.begin(); it != client_channels.end(); ++it) {
        std::map<std::string, Channel>::iterator chit = all_channels.find(*it);
        if (chit == all_channels.end())
            continue;
        recipients.insert(chit->second.getMembers().begin(), chit->second.getMembers().end());
    }
    return recipients;
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
        _server.sendToClient(client.getFd(), formReply(ERR_NONICKNAMEGIVEN, client));
        return;
    }
    if (!isValidNickname(message->parameters[0])){
        _server.sendToClient(client.getFd(), formReply(ERR_ERRONEUSNICKNAME, message->parameters[0], client));
        return;
    }
    if (nickInUse(message->parameters[0], _server.getClients())) {
        _server.sendToClient(client.getFd(), formReply(ERR_NICKNAMEINUSE, message->parameters[0], client));
        return;
    }
    if (client.isRegistered()) {
        std::string nick_change_msg = ":" + client.getNickname() + " NICK " + message->parameters[0] + CRLF;
        std::set<int> recipients = getRecipients(client.getChannels(), _server.getChannels());
        for (std::set<int>::iterator it = recipients.begin(); it != recipients.end(); ++it) {
            if (*it != client.getFd())
                _server.sendToClient(*it, nick_change_msg);
        }
        _server.sendToClient(client.getFd(), nick_change_msg);
    }
    client.setNickname(message->parameters[0]);
    tryToRegister(client);
}

void CommandHandler::handleUser(s_msg *message, Client& client) {
    if (client.isRegistered()) {
        _server.sendToClient(client.getFd(), formReply(ERR_ALREADYREGISTRED, client));
        return;
    }
    else if (message->parameters.size() < 4) {
        _server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, message->command, client));
        return;
    }
    client.setUsername(message->parameters[0]);
    tryToRegister(client);
}

static std::string makePrefix(Client& client) {
    return ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHost();
}

void CommandHandler::handleQuit(s_msg* message, Client& client) {

    std::string reason;
    if (!message->parameters.empty())
        reason = message->parameters[0];
    else
        reason = "Client Quit";

    std::string quit_msg = makePrefix(client) + " QUIT :" + reason + CRLF;

    std::set<int> recipients = getRecipients(client.getChannels(), _server.getChannels());

    for (std::set<int>::iterator it = recipients.begin(); it != recipients.end(); ++it) {
        if (*it != client.getFd())
            _server.sendToClient(*it, quit_msg);
    }
    _server.disconnectClient(client, reason);
}

static bool isChannelName(const std::string& name) {
    if (name.empty())
        return false;
    if (name[0] != '#' && name[0] != '&')
        return false;
    if (name.size() > 200)
        return false;
    for (size_t i = 0; i < name.size(); ++i) {
        if (name[i] == ' ' || name[i] == ',' || name[i] == 7)
            return false;
    }
    return true;
}

static std::string getNamesList(Channel& channel, Server& server) {
    std::ostringstream oss;
    const std::set<int>& members = channel.getMembers();
    for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
        std::map<int, Client> clients = server.getClients();
        std::map<int, Client>::iterator client_it = clients.find(*it);
        if (client_it != clients.end()) {
            if (oss.tellp() > 0)
                oss << " ";
			if (channel.isOperator(client_it->first))
				oss << "@";
            oss << client_it->second.getNickname();
        }
    }
    return oss.str();
}

std::vector<std::string> splitParameters(const std::string& param) {
	std::vector<std::string> result;
	std::istringstream iss(param);
	std::string token;
	while (std::getline(iss, token, ',')) {
		result.push_back(token);
	}
	return result;
}

void CommandHandler::handleJoin(s_msg *message, Client& client) {
    if (!client.isRegistered()) {
        _server.sendToClient(client.getFd(), formReply(ERR_NOTREGISTERED, client));
        return;
    }
    if (message->parameters.empty()) {
        _server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, message->command, client));
        return;
    }

    std::vector<std::string> channel_names = splitParameters(message->parameters[0]);
	
	for (std::vector<std::string>::iterator it = channel_names.begin(); it != channel_names.end(); ++it) {
			
		std::string channel_name = *it;

		if (!isChannelName(channel_name)) {
			_server.sendToClient(client.getFd(), formReply(ERR_NOSUCHCHANNEL, channel_name, client));
			return;
		}

		std::map<std::string, Channel>& channels = _server.getChannels();
		if (channels.find(channel_name) == channels.end())
		{
			channels[channel_name] = Channel(channel_name);
			channels[channel_name].addOperator(client.getFd());
		}

		Channel& channel = channels[channel_name];
		if (channel.hasMember(client.getFd()))
			return;

		channel.addMember(client.getFd());
        client.getChannels().insert(channel_name);

		std::string join_msg = makePrefix(client) + " JOIN :" + channel_name + CRLF;
		_server.broadcastToChannel(channel_name, join_msg, -1);

		std::string names = getNamesList(channel, _server);
		std::string rpl_names = ":" + std::string(SERVER) + " 353 " + client.getNickname() + " = " + channel_name + " :" + names + CRLF;
		std::string rpl_end = ":" + std::string(SERVER) + " 366 " + client.getNickname() + " " + channel_name + " :End of /NAMES list" + CRLF;
		_server.sendToClient(client.getFd(), rpl_names);
		_server.sendToClient(client.getFd(), rpl_end);
	}
}

void CommandHandler::handlePrivmsg(s_msg *message, Client& client) {
    if (!client.isRegistered()) {
        _server.sendToClient(client.getFd(), formReply(ERR_NOTREGISTERED, client));
        return;
    }

    bool automatic_reply = message->command == "NOTICE" ? false : true;

    if (message->parameters.empty()) {
        if (automatic_reply)
            _server.sendToClient(client.getFd(), formReply(ERR_NORECIPIENT, message->command, client));
        return;
    }
    if (message->parameters.size() < 2 || message->parameters[1].empty()) {
        if (automatic_reply)
            _server.sendToClient(client.getFd(), formReply(ERR_NOTEXTTOSEND, client));
        return;
    }

    std::vector<std::string> targets = splitParameters(message->parameters[0]);
    std::string text = message->parameters[1];

   for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); ++it) {
		std::string target = *it;

		if (target.empty())
			continue;

		std::string msg = makePrefix(client) + " " + message->command + " "
			+ target + " :" + text + CRLF;

		if (target[0] == '#' || target[0] == '&') {
			std::map<std::string, Channel>& channels = _server.getChannels();
			std::map<std::string, Channel>::iterator chan_it = channels.find(target);

			if (chan_it == channels.end()) {
                if (automatic_reply)
				    _server.sendToClient(client.getFd(), formReply(ERR_NOSUCHCHANNEL, target, client));
				continue;
			}

			if (!chan_it->second.hasMember(client.getFd())) {
                if (automatic_reply)
				    _server.sendToClient(client.getFd(), formReply(ERR_CANNOTSENDTOCHAN, target, client));
				continue;
			}

			_server.broadcastToChannel(target, msg, client.getFd());
			continue;
		}

		Client* target_client = _server.getClientByNickname(target);

		if (!target_client) {
            if (automatic_reply)
			    _server.sendToClient(client.getFd(), formReply(ERR_NOSUCHNICK, target, client));
			continue;
		}

		_server.sendToClient((*target_client).getFd(), msg);
	}
}

void CommandHandler::handleMode(s_msg *message, Client& client) {
	if (!client.isRegistered()) {
		_server.sendToClient(client.getFd(), formReply(ERR_NOTREGISTERED, client));
		return;
	}
	
	if (message->parameters.empty()) {
		_server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, message->command, client));
		return;
	}

    std::map<std::string, Channel>& channels = _server.getChannels();
    std::map<std::string, Channel>::iterator it = channels.find(message->parameters[0]);
    if (it == channels.end()) {
        _server.sendToClient(client.getFd(), formReply(ERR_NOSUCHCHANNEL, message->parameters[0], client));
        return;
    }

    Channel& thisChannel = it->second;
	if (!thisChannel.isOperator(client.getFd())) {
		_server.sendToClient(client.getFd(), formReply(ERR_CHANOPRIVSNEEDED, message->parameters[0], client));
		return;
	}

	if (message->parameters.size() < 2) {
		_server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, message->command, client));
		return;
	}

	std::string target = message->parameters[0];
	std::cout << "Mode command received for target: " << target << std::endl;

	if (message->parameters[1][0] == '+') {
		if (message->parameters[1].find('o') != std::string::npos) {
			if (message->parameters.size() < 3) {
				_server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, message->command, client));
				return;
			}
			std::string nick_to_op = message->parameters[2];
			Client* target_client = _server.getClientByNickname(nick_to_op);
			if (!target_client) {
				_server.sendToClient(client.getFd(), formReply(ERR_NOSUCHNICK, nick_to_op, client));
				return;
			}
			thisChannel.addOperator(target_client->getFd());
		} else if (message->parameters[1].find('i') != std::string::npos) {
			thisChannel.setInviteOnly(true);
		} else if (message->parameters[1].find('k') != std::string::npos) {
			thisChannel.setPassword(message->parameters[2]);
		} else if (message->parameters[1].find('l') != std::string::npos) {
			if (message->parameters.size() < 3) {
				_server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, message->command, client));
				return;
			}
			int limit = std::atoi(message->parameters[2].c_str());
			thisChannel.setMemberLimit(limit);
		} else if (message->parameters[1].find('t') != std::string::npos) {
			thisChannel.setRestrictedTopic(true);
		} else {
			_server.sendToClient(client.getFd(), formReply(ERR_UNKNOWNMODE, message->parameters[1], client));
			return;
		}
	} else if (message->parameters[1][0] == '-') {
		if (message->parameters[1].find('o') != std::string::npos) {
			if (message->parameters.size() < 3) {
				_server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, message->command, client));
				return;
			}
			std::string nick_to_deop = message->parameters[2];
			Client* target_client = _server.getClientByNickname(nick_to_deop);
			if (!target_client) {
				_server.sendToClient(client.getFd(), formReply(ERR_NOSUCHNICK, nick_to_deop, client));
				return;
			}
			thisChannel.removeOperator(target_client->getFd());
		} else if (message->parameters[1].find('i') != std::string::npos) {
			thisChannel.setInviteOnly(false);
		} else if (message->parameters[1].find('k') != std::string::npos) {
			thisChannel.removePassword();
		} else if (message->parameters[1].find('l') != std::string::npos) {
			thisChannel.setMemberLimit(-1); 
		} else if (message->parameters[1].find('t') != std::string::npos) {
			thisChannel.setRestrictedTopic(false);
		} else {
			_server.sendToClient(client.getFd(), formReply(ERR_UNKNOWNMODE, message->parameters[1], client));
			return;
		}
	} else {
		_server.sendToClient(client.getFd(), formReply(ERR_UNKNOWNMODE, message->parameters[1], client));
		return;
	}

}

void CommandHandler::handleInvite(s_msg *message, Client& client) {
	if (!client.isRegistered()) {
		_server.sendToClient(client.getFd(), formReply(ERR_NOTREGISTERED, client));
		return;
	}

	if (message->parameters.size() < 2) {
		_server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, message->command, client));
		return;
	}

	std::string nick_to_invite = message->parameters[0];
	std::string channel_name = message->parameters[1];

	Client* target_client = _server.getClientByNickname(nick_to_invite);
	if (!target_client) {
		_server.sendToClient(client.getFd(), formReply(ERR_NOSUCHNICK, nick_to_invite, client));
		return;
	}

	std::map<std::string, Channel>& channels = _server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channel_name);
	if (it == channels.end()) {
		_server.sendToClient(client.getFd(), formReply(ERR_NOSUCHCHANNEL, channel_name, client));
		return;
	}

	Channel& thisChannel = it->second;
	if (!thisChannel.hasMember(client.getFd())) {
		_server.sendToClient(client.getFd(), formReply(ERR_NOTONCHANNEL, channel_name, client));
		return;
	}

	size_t limit = thisChannel.getMemberLimit();
	if (thisChannel.hasLimit() && thisChannel.getMembers().size() >= limit) {
		_server.sendToClient(client.getFd(), formReply(ERR_CHANNELISFULL, channel_name, client));
		return;
	}

	thisChannel.addMember(target_client->getFd());
	target_client->getChannels().insert(channel_name);

	std::string join_msg = makePrefix(*target_client) + " JOIN :" + channel_name + CRLF;
	std::string invite_msg = makePrefix(client) + " INVITE " + nick_to_invite + " :" + channel_name + CRLF;

	_server.broadcastToChannel(channel_name, join_msg, -1);
	_server.sendToClient(target_client->getFd(), invite_msg);
}

void CommandHandler::handleTopic(s_msg *message, Client& client) {
	if (!client.isRegistered()) {
		_server.sendToClient(client.getFd(), formReply(ERR_NOTREGISTERED, client));
		return;
	}

	if (message->parameters.size() < 2) {
		_server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, message->command, client));
		return;
	}

	std::map<std::string, Channel>& channels = _server.getChannels();
    std::map<std::string, Channel>::iterator it = channels.find(message->parameters[0]);
    if (it == channels.end()) {
        _server.sendToClient(client.getFd(), formReply(ERR_NOSUCHCHANNEL, message->parameters[0], client));
        return;
    }

    Channel& thisChannel = it->second;
	if (!thisChannel.isOperator(client.getFd())) {
		_server.sendToClient(client.getFd(), formReply(ERR_CHANOPRIVSNEEDED, message->parameters[0], client));
		return;
	}

	if (message->parameters.size() == 2) {
		thisChannel.setTopic(message->parameters[1]);
		std::string topic_msg = makePrefix(client) + " TOPIC " + message->parameters[0] + " :" + message->parameters[1] + CRLF;
		_server.broadcastToChannel(message->parameters[0], topic_msg, -1);
	} else {
		_server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, message->command, client));
		return;
	}

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
    else if (message->command == "JOIN")
        handleJoin(message, client);
    else if (message->command == "PRIVMSG" || message->command == "NOTICE")
        handlePrivmsg(message, client);
	else if (message->command == "MODE")
		handleMode(message, client);
	else if (message->command == "INVITE")
		handleInvite(message, client);
	// else if (message->command == "KICK")
	// 	handleKick(message, client);
	else if (message->command == "TOPIC")
		handleTopic(message, client);
	else
		_server.sendToClient(client.getFd(), formReply(ERR_UNKNOWNMODE, message->command, client));
}

