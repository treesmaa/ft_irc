#include "CommandHandler.hpp"
#include "Server.hpp"
//important: '%s' should be removed from the reply string!
//it's just to show which replies take additional parameters.
std::map<int, std::string> initReplies() {
    std::map<int, std::string> r;
	// Ping
	r[ERR_NOORIGIN]			= " :No origin specified";					//409

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

	// MODE success
	r[RPL_CHANNELMODEIS]      = "";                                        // 324
	r[RPL_INVITING]           = " :Inviting";                              // 341
	r[RPL_WHOREPLY]			 = " :Who reply";							  // 352
	r[RPL_ENDOFWHO]			 = " :End of /WHO list";					  // 315
	r[RPL_WHOISUSER]		 = " :Whois user";							  // 311
	r[RPL_ENDOFWHOIS]		 = " :End of /WHOIS list";					  // 318
	r[RPL_BANNLIST]			 = " :End of channel's ban list";			// 368

    // Channel operator permissions
    r[ERR_CHANOPRIVSNEEDED]   = " :You're not channel operator";           // 482

    // Messaging
    r[ERR_NORECIPIENT]        = " :No recipient given";                  // 411
    r[ERR_NOTEXTTOSEND]       = " :No text to send";                          // 412
    r[ERR_CANNOTSENDTOCHAN]   = " :Cannot send to channel";                // 404
    r[ERR_NOSUCHNICK]         = " :No such nick/channel";                  // 401

    // MODE
    r[ERR_UNKNOWNMODE]        = " :No such mode modifier";            // 472
	r[ERR_NOTNUMBER]          = " :Parameter is not a number";            // 696
	r[ERR_OVERFLOW]           = " :Parameter is too large";            // 697

    // INVITE
    r[ERR_NOTONCHANNEL]       = " :You're not on that channel";            // 442

	//UNKNOWN COMMAND
	r[ERR_UNKNOWNCOMMAND]       = " :Unknown command";                     // 421

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
		|| code == ERR_NOTONCHANNEL || code == ERR_CHANOPRIVSNEEDED || code == ERR_NOTONCHANNEL || code == RPL_CHANNELMODEIS
		|| code == RPL_INVITING || code == RPL_WHOREPLY || code == RPL_ENDOFWHO || code == RPL_WHOISUSER || code == RPL_ENDOFWHOIS
		|| code == RPL_BANNLIST)
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
	if (code == ERR_USERONCHANNEL)
		oss << ":" << SERVER << " " << code << " " << client.getNickname() << " " << nick << " " << command << _replies[code] << CRLF;
	else if (code == ERR_USERNOTINCHANNEL)
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

static std::string makeUserReply(int code, const Client& requester, const Client& target) {
	std::ostringstream oss;
	oss << ":" << SERVER << " " << code << " " << requester.getNickname()
		<< " " << target.getNickname() << "!" << target.getUsername()
		<< "@" << target.getHost() << " :Whois user" << CRLF;
	return oss.str();
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

static std::string getChannelModes(Channel& channel) {
	std::ostringstream oss;
	oss << "+";
	if (channel.isInviteOnly())
		oss << "i";
	if (channel.hasRestrictedTopic())
		oss << "t";
	if (channel.hasPassword())
		oss << "k";
	if (channel.hasLimit())
		oss << "l";
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
	std::vector<std::string> keys;

	if (message->parameters.size() > 1)
		keys = splitParameters(message->parameters[1]);

	for (size_t i = 0; i < channel_names.size(); ++i) {
		std::string channel_name = channel_names[i];
		std::string provided_key;

		if (i < keys.size())
			provided_key = keys[i];

		if (!isChannelName(channel_name)) {
			_server.sendToClient(
				client.getFd(),
				formReply(ERR_NOSUCHCHANNEL, channel_name, client)
			);
			continue;
		}

		std::map<std::string, Channel>& channels = _server.getChannels();
		bool new_channel = false;

		if (channels.find(channel_name) == channels.end()) {
			channels[channel_name] = Channel(channel_name);
			channels[channel_name].addOperator(client.getFd());
			new_channel = true;
		}

		Channel& channel = channels[channel_name];

		if (channel.hasMember(client.getFd())) {
			continue;
		}

		if (!new_channel
			&& channel.hasPassword()
			&& provided_key != channel.getPassword()) {
			_server.sendToClient(
				client.getFd(),
				formReply(ERR_BADCHANNELKEY, channel_name, client)
			);
			continue;
		}

		if (!new_channel
			&& channel.isInviteOnly()
			&& !channel.isInvited(client.getFd())) {
			_server.sendToClient(
				client.getFd(),
				formReply(ERR_INVITEONLYCHAN, channel_name, client)
			);
			continue;
		}

		if (!new_channel
			&& channel.hasLimit()
			&& (int)channel.getMembers().size() >= channel.getMemberLimit()) {
			_server.sendToClient(
				client.getFd(),
				formReply(ERR_CHANNELISFULL, channel_name, client)
			);
			continue;
		}

		channel.addMember(client.getFd());
		channel.removeInvitedUser(client.getFd());
		client.getChannels().insert(channel_name);

		std::string join_msg =
			makePrefix(client) + " JOIN :" + channel_name + CRLF;

		_server.broadcastToChannel(channel_name, join_msg, -1);

		std::string names = getNamesList(channel, _server);

		std::string rpl_names =
			":" + std::string(SERVER) + " 353 "
			+ client.getNickname() + " = "
			+ channel_name + " :" + names + CRLF;

		std::string rpl_end =
			":" + std::string(SERVER) + " 366 "
			+ client.getNickname() + " "
			+ channel_name + " :End of /NAMES list" + CRLF;

		_server.sendToClient(client.getFd(), rpl_names);
		_server.sendToClient(client.getFd(), rpl_end);
	}
}

void CommandHandler::handlePart(s_msg *message, Client& client) {
	if (!client.isRegistered()) {
        _server.sendToClient(client.getFd(), formReply(ERR_NOTREGISTERED, client));
        return;
    }
    if (message->parameters.empty()) {
        _server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, message->command, client));
        return;
    }
	std::vector<std::string> channel_names = splitParameters(message->parameters[0]);

	for (size_t i = 0; i < channel_names.size(); ++i) {
		std::string channel_name = channel_names[i];

		if (!isChannelName(channel_name)) {
			_server.sendToClient(
				client.getFd(),
				formReply(ERR_NOSUCHCHANNEL, channel_name, client)
			);
			continue;
		}

		std::map<std::string, Channel>& channels = _server.getChannels();
		std::map<std::string, Channel>::iterator it = channels.find(channel_name);
		if (it == channels.end()) {
			_server.sendToClient(client.getFd(), formReply(ERR_NOTONCHANNEL, message->command, client));
			continue;
		}
		if (!channels[channel_name].hasMember(client.getFd())) {
			_server.sendToClient(client.getFd(), formReply(ERR_NOTONCHANNEL, message->command, client));
			continue;
		}

		std::string part_msg = makePrefix(client) + " PART " + channel_name + CRLF;
		_server.broadcastToChannel(channel_name, part_msg, -1);
		channels[channel_name].removeMember(client.getFd());
		client.getChannels().erase(channel_name);

		if (channels[channel_name].isEmpty())
			channels.erase(channel_name);
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

void CommandHandler::sendModeSuccess(const std::string& mode, Client& client, const std::string& target) {
		std::string mode_msg = makePrefix(client) + " MODE " + target + " " + mode + CRLF;
		_server.broadcastToChannel(target, mode_msg, -1);
	}

void CommandHandler::handleMode(s_msg *message, Client& client) {
	if (!client.isRegistered()) {
		_server.sendToClient(
			client.getFd(),
			formReply(ERR_NOTREGISTERED, client)
		);
		return;
	}

	if (message->parameters.empty()) {
		_server.sendToClient(
			client.getFd(),
			formReply(ERR_NEEDMOREPARAMS, message->command, client)
		);
		return;
	}

	std::string target = message->parameters[0];

	if (!isChannelName(target)) {
		// User MODE is not implemented.
		return;
	}

	std::map<std::string, Channel>& channels = _server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(target);

	if (it == channels.end()) {
		_server.sendToClient(
			client.getFd(),
			formReply(ERR_NOSUCHCHANNEL, target, client)
		);
		return;
	}

	Channel& thisChannel = it->second;
	std::string appliedModes = getChannelModes(thisChannel);

	// MODE #channel
	if (message->parameters.size() < 2) {
		if (appliedModes == "+") {
			_server.sendToClient(
				client.getFd(),
				formReply(RPL_CHANNELMODEIS, target, client)
			);
		} else {
			_server.sendToClient(
				client.getFd(),
				formReply(
					RPL_CHANNELMODEIS,
					target + " " + appliedModes,
					client
				)
			);
		}
		return;
	}

	std::string modes = message->parameters[1];

	// Ban list is not implemented.
	if (modes == "b" || modes == "+b") {
		_server.sendToClient(
			client.getFd(),
			formReply(RPL_BANNLIST, target, client)
		);
		return;
	}

	if (!thisChannel.isOperator(client.getFd())) {
		_server.sendToClient(
			client.getFd(),
			formReply(ERR_CHANOPRIVSNEEDED, target, client)
		);
		return;
	}

	bool adding = true;
	size_t keyWord = 2;

	for (size_t i = 0; i <= MAX_MODES; ++i) {
		char mode = modes[i];

		if (mode == '+') {
			adding = true;
			continue;
		}

		if (mode == '-') {
			adding = false;
			continue;
		}

		if (mode == 'i') {
			thisChannel.setInviteOnly(adding);
			sendModeSuccess(adding ? "+i" : "-i", client, target);
			continue;
		}

		if (mode == 't') {
			thisChannel.setRestrictedTopic(adding);
			sendModeSuccess(adding ? "+t" : "-t", client, target);
			continue;
		}

		if (mode == 'k') {
			if (adding) {
				if (keyWord >= message->parameters.size()) {
					_server.sendToClient(
						client.getFd(),
						formReply(
							ERR_NEEDMOREPARAMS,
							message->command,
							client
						)
					);
					continue;
				}

				std::string password = message->parameters[keyWord++];

				thisChannel.setPassword(password);
				sendModeSuccess(
					"+k " + password,
					client,
					target
				);
			} else {
				
				thisChannel.removePassword();
				sendModeSuccess("-k", client, target);
			}

			continue;
		}

		if (mode == 'o') {
			if (keyWord >= message->parameters.size()) {
				_server.sendToClient(
					client.getFd(),
					formReply(
						ERR_NEEDMOREPARAMS,
						message->command,
						client
					)
				);
				continue;
			}

			
			std::string nickname = message->parameters[keyWord++];
			Client* targetClient =
				_server.getClientByNickname(nickname);

			if (!targetClient) {
				_server.sendToClient(
					client.getFd(),
					formReply(ERR_NOSUCHNICK, nickname, client)
				);
				continue;
			}

			if (!thisChannel.hasMember(targetClient->getFd())) {
				_server.sendToClient(
					client.getFd(),
					formReply(
						ERR_USERNOTINCHANNEL,
						nickname,
						target,
						client
					)
				);
				continue;
			}

			if (adding) {
				thisChannel.addOperator(targetClient->getFd());
				sendModeSuccess(
					"+o " + nickname,
					client,
					target
				);
			} else {
				thisChannel.removeOperator(targetClient->getFd());
				sendModeSuccess(
					"-o " + nickname,
					client,
					target
				);
			}

			continue;
		}

		if (mode == 'l') {
			if (!adding) {
				thisChannel.setMemberLimit(-1);
				sendModeSuccess("-l", client, target);
				continue;
			}

			if (keyWord >= message->parameters.size()) {
				_server.sendToClient(
					client.getFd(),
					formReply(
						ERR_NEEDMOREPARAMS,
						message->command,
						client
					)
				);
				continue;
			}

			std::string limitString =
				message->parameters[keyWord++];

			bool validNumber = !limitString.empty();

			for (size_t j = 0; j < limitString.size(); ++j) {
				unsigned char character =
					static_cast<unsigned char>(limitString[j]);

				if (!std::isdigit(character)) {
					validNumber = false;
					break;
				}
			}

			if (!validNumber) {
				_server.sendToClient(
					client.getFd(),
					formReply(
						ERR_NOTNUMBER,
						message->command,
						client
					)
				);
				continue;
			}

			int limit = std::atoi(limitString.c_str());

			if (limit <= 0) {
				_server.sendToClient(
					client.getFd(),
					formReply(
						ERR_OVERFLOW,
						message->command,
						client
					)
				);
				continue;
			}

			thisChannel.setMemberLimit(limit);
			sendModeSuccess(
				"+l " + limitString,
				client,
				target
			);
			continue;
		}

		std::string unknownMode(1, mode);

		_server.sendToClient(
			client.getFd(),
			formReply(ERR_UNKNOWNMODE, unknownMode, client)
		);
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

	if (!thisChannel.isOperator(client.getFd())) {
		_server.sendToClient(client.getFd(), formReply(ERR_CHANOPRIVSNEEDED, channel_name, client));
		return;
	}
	
	if (thisChannel.hasMember(target_client->getFd())) {
		_server.sendToClient(client.getFd(), formReply(ERR_USERONCHANNEL, nick_to_invite, channel_name, client));
		return;
	}

	thisChannel.addInvitedUser(target_client->getFd());

	std::string invite_msg = makePrefix(client) + " INVITE " + nick_to_invite + " " + channel_name + CRLF;
	_server.sendToClient(target_client->getFd(), invite_msg);
	_server.sendToClient(client.getFd(), formReply(RPL_INVITING, nick_to_invite, channel_name, client));
}

void CommandHandler::handleTopic(s_msg *message, Client& client) {
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
	if (message->parameters.size() < 2) {
		if (!thisChannel.getTopic().empty()) {
			std::string rpl_topic = ":" + std::string(SERVER) + " 332 " + client.getNickname()
				+ " " + message->parameters[0] + " :" + thisChannel.getTopic() + CRLF;
			_server.sendToClient(client.getFd(), rpl_topic);
		} else {
			std::string rpl_notopic = ":" + std::string(SERVER) + " 331 " + client.getNickname()
				+ " " + message->parameters[0] + " :No topic is set" + CRLF;
			_server.sendToClient(client.getFd(), rpl_notopic);
		}
		return;
	}

	if (!thisChannel.hasMember(client.getFd())) {
		_server.sendToClient(client.getFd(), formReply(ERR_NOTONCHANNEL, message->parameters[0], client));
		return;
	}

	if (thisChannel.hasRestrictedTopic() && !thisChannel.isOperator(client.getFd())) {
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

void CommandHandler::handleKick(s_msg *message, Client& client) {
	if (!client.isRegistered()) {
		_server.sendToClient(client.getFd(), formReply(ERR_NOTREGISTERED, client));
		return;
	}

	if (message->parameters.size() < 2) {
		_server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, message->command, client));
		return;
	}

	std::string channel_name = message->parameters[0];
	std::string nick_to_kick = message->parameters[1];
	std::string reason = (message->parameters.size() >= 3) ? message->parameters[2] : "No reason specified";

	std::map<std::string, Channel>& channels = _server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channel_name);
	if (it == channels.end()) {
		_server.sendToClient(client.getFd(), formReply(ERR_NOSUCHCHANNEL, channel_name, client));
		return;
	}

	Channel& thisChannel = it->second;
	if (!thisChannel.isOperator(client.getFd())) {
		_server.sendToClient(client.getFd(), formReply(ERR_CHANOPRIVSNEEDED, channel_name, client));
		return;
	}

	Client* target_client = _server.getClientByNickname(nick_to_kick);
	if (!target_client) {
		_server.sendToClient(client.getFd(), formReply(ERR_NOSUCHNICK, nick_to_kick, client));
		return;
	}

	if (!thisChannel.hasMember(target_client->getFd())) {
		_server.sendToClient(client.getFd(), formReply(ERR_USERNOTINCHANNEL, nick_to_kick, channel_name, client));
		return;
	}

	std::string kick_msg = makePrefix(client) + " KICK " + channel_name + " " + nick_to_kick + " :" + reason + CRLF;
	thisChannel.removeMember(target_client->getFd());
	target_client->getChannels().erase(channel_name);

	_server.sendToClient(target_client->getFd(), kick_msg);
	_server.broadcastToChannel(channel_name, kick_msg, target_client->getFd());

	if (thisChannel.isEmpty())
		channels.erase(it);
}

void CommandHandler::handlePing(s_msg *message, Client& client) {
    if (message->parameters.empty()) {
        _server.sendToClient(client.getFd(), formReply(ERR_NOORIGIN, client));
        return;
    }
	_server.sendToClient(client.getFd(), "PONG " + message->parameters[0] + CRLF);
}

void CommandHandler::handleCAP(s_msg *message, Client& client) {
	if (message->parameters.empty()) {
		_server.sendToClient(client.getFd(), formReply(ERR_NEEDMOREPARAMS, message->command, client));
		return;
	}
	if (message->parameters[0] == "LS")
		_server.sendToClient(client.getFd(), ":" SERVER " CAP * LS :" CRLF);
}

static std::string makeWhoReply(Client& requester, const Client& target, const std::string& channel) {
	std::ostringstream oss;
	oss << ":" << SERVER << " " << RPL_WHOREPLY << " " << requester.getNickname()
		<< " " << channel << " " << target.getUsername() << " "
		<< target.getHost() << " " << SERVER << " "
		<< target.getNickname() << " H :0 " << target.getUsername() << CRLF;
	return oss.str();
}

void CommandHandler::handleWho(s_msg *message, Client& client) {
	if (!client.isRegistered()) {
		_server.sendToClient(
			client.getFd(),
			formReply(ERR_NOTREGISTERED, client)
		);
		return;
	}

	if (message->parameters.empty()) {
		_server.sendToClient(
			client.getFd(),
			formReply(ERR_NEEDMOREPARAMS, message->command, client)
		);
		return;
	}

	std::string target = message->parameters[0];

	std::map<std::string, Channel>& channels = _server.getChannels();
	std::map<std::string, Channel>::iterator channelIt =
		channels.find(target);

	if (channelIt == channels.end()) {
		_server.sendToClient(
			client.getFd(),
			formReply(ERR_NOSUCHCHANNEL, target, client)
		);
		return;
	}

	const std::set<int>& members = channelIt->second.getMembers();
	const std::map<int, Client>& clients = _server.getClients();

	for (std::set<int>::const_iterator memberIt = members.begin();
		 memberIt != members.end();
		 ++memberIt) {

		std::map<int, Client>::const_iterator clientIt =
			clients.find(*memberIt);

		if (clientIt == clients.end())
			continue;

		const Client& targetClient = clientIt->second;

		_server.sendToClient(
			client.getFd(),
			makeWhoReply(client, targetClient, target)
		);
	}

	_server.sendToClient(
		client.getFd(),
		formReply(RPL_ENDOFWHO, target, client)
	);
}


void CommandHandler::handleWhois(s_msg *message, Client& client) {
	if (!client.isRegistered()) {
		_server.sendToClient(
			client.getFd(),
			formReply(ERR_NOTREGISTERED, client)
		);
		return;
	}

	if (message->parameters.empty()) {
		_server.sendToClient(
			client.getFd(),
			formReply(ERR_NEEDMOREPARAMS, message->command, client)
		);
		return;
	}

	std::string nickname = message->parameters[0];

	Client* target_client = _server.getClientByNickname(nickname);
	if (!target_client) {
		_server.sendToClient(
			client.getFd(),
			formReply(ERR_NOSUCHNICK, nickname, client)
		);
		return;
	}

	_server.sendToClient(
		client.getFd(),
		makeUserReply(RPL_WHOISUSER, client, *target_client)
	);

	_server.sendToClient(
		client.getFd(),
		formReply(RPL_ENDOFWHOIS, nickname, client)
	);
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
	else if (message->command == "PART")
		handlePart(message, client);
    else if (message->command == "PRIVMSG" || message->command == "NOTICE")
        handlePrivmsg(message, client);
	else if (message->command == "MODE")
		handleMode(message, client); 
	else if (message->command == "INVITE")
		handleInvite(message, client);
	else if (message->command == "KICK")
		handleKick(message, client);
	else if (message->command == "TOPIC")
		handleTopic(message, client);
	else if (message->command == "PING")
		handlePing(message, client);
	else if (message->command == "CAP")
		handleCAP(message, client);
	else if (message->command == "WHO")
		handleWho(message, client);
	else if (message->command == "WHOIS")
		handleWhois(message, client);
	else
		_server.sendToClient(client.getFd(), formReply(ERR_UNKNOWNCOMMAND, message->command, client));
}

