#include "Server.hpp"

Server::Server() {}

Server::Server(int port, char *password)
    :   _port(port),
        _password(password),
        _server_fd(-1),
        _creation_date(getCurrentDate()) {}

Server::~Server() {
    if (_server_fd != -1)
        close(_server_fd);
    for (std::vector<pollfd>::iterator it = _pfds.begin(); it != _pfds.end(); ++it) {
        if (it->fd != -1)
            close(it->fd);
    }
}

std::string Server::getCreationDate() const {
    return _creation_date;
}

std::string Server::getPassword() const {
    return _password;
}

std::map<int, Client> Server::getClients() const {
    return _clients;
}

std::map<std::string, Channel>& Server::getChannels() {
    return _channels;
}

Client* Server::getClientByNickname(const std::string& nickname) {
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second.getNickname() == nickname)
            return &(it->second);
    }
    return NULL;
}

std::string Server::getCurrentDate() const {
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
	return buf;
}

void Server::sendToClient(int fd, const std::string& message) {
    if (fd < 0)
        return;
    DEBUG_PRINT("[send] ", fd, message);
    if (send(fd, message.c_str(), message.size(), 0) == -1)
        std::cerr << "Send error" << std::endl;
}

void Server::broadcastToChannel(const std::string& channel_name, const std::string& message, int except_fd) {
    std::map<std::string, Channel>::iterator chan_it = _channels.find(channel_name);
    if (chan_it == _channels.end())
        return;

    const std::set<int>& members = chan_it->second.getMembers();
    for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
        if (*it != except_fd)
            sendToClient(*it, message);
    }
}

void Server::removeClientFromChannels(int fd) {
    std::map<std::string, Channel>::iterator it = _channels.begin();
    while (it != _channels.end()) {
        it->second.removeMember(fd);
        if (it->second.isEmpty())
            _channels.erase(it++);
        else
            ++it;
    }
}

std::string intToString(int n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

void Server::addToPoll(int fd) {
    struct pollfd p;
    p.fd = fd;
    p.events = POLLIN;
    p.revents = 0;
    _pfds.push_back(p);
}

void Server::removePollFd(int client_fd) {
    for (std::vector<pollfd>::iterator it = _pfds.begin(); it != _pfds.end(); ++it) {
        if (it->fd == client_fd) {
            _pfds.erase(it);
            break;
        } 
    }
}

void Server::removeClient(int fd) {
    removeClientFromChannels(fd);
    removePollFd(fd);
    _clients.erase(fd);
    close(fd);
}

void Server::serverSocketSetup() {
    struct addrinfo hints;
    struct addrinfo *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, intToString(_port).c_str(), &hints, &servinfo);
    if ((status != 0))
        throw std::runtime_error(gai_strerror(status));
    
    struct addrinfo *p;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        _server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (_server_fd == -1)
            continue;
        int yes = 1;
        if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket options: ") + strerror(errno));
        }
        //to detect dead clients (disconnected without sending a QUIT)
        if (setsockopt(_server_fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket options: ") + strerror(errno));
        }
        //set socket fd in non-blocking mode
        if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket to non-blocking: ") + strerror(errno));
        }
        if (bind(_server_fd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        
        close(_server_fd);
        _server_fd = -1;
    }

    freeaddrinfo(servinfo);

    if (p == NULL)
        throw std::runtime_error(std::string("failed to bind socket: ") + strerror(errno));

    if (listen(_server_fd, SOMAXCONN) == -1)
        throw std::runtime_error(std::string("failed to listen on socket: ") + strerror(errno));

    addToPoll(_server_fd);
    addToPoll(STDIN_FILENO);
    std::cout << "IRC server listening on port " << _port << std::endl;
}

void Server::acceptNewClient() {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int new_fd = accept(_server_fd, (struct sockaddr *)&client_addr, &addr_size);
    if (new_fd == -1) {
        std::cerr << "Error: failed to accept client" << std::endl;
        return;
    }
    if (fcntl(new_fd, F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "Error: fcntl(): " << strerror(errno) << std::endl;
        close(new_fd);
        return;
    }
    //getting hostname for new client
    char ipstr[INET6_ADDRSTRLEN];
    const char *printable_addr;
    if (client_addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&client_addr;
        printable_addr = inet_ntop(client_addr.ss_family, &s->sin_addr, ipstr, sizeof(ipstr));
    }
    else {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&client_addr;
        printable_addr = inet_ntop(client_addr.ss_family, &s->sin6_addr, ipstr, sizeof(ipstr));
    }
    if (!printable_addr) {
        std::cerr << "Error: inet_ntop() failed: " << strerror(errno) << std::endl;
    }
    else {
        std::cout << std::left << std::setw(14) << "[connect]" << " fd=" << new_fd << " host=" << ipstr << std::endl;
        _clients[new_fd] = Client(new_fd, ipstr);
        addToPoll(new_fd);
    }
}

void Server::disconnectClient(Client & client, std::string reason) {
    int client_fd = client.getFd();
    std::cout << std::left << std::setw(14) << "[disconnect]" << " fd=" << client_fd << " host=" << client.getHost() << " nickname=" << client.getNickname() << " reason=" << reason << std::endl;
    removeClient(client_fd);
}

void Server::handleMessage(std::string& line, Client& client) {
    s_msg message;
    if (parse(line, message) == -1) {
        std::cerr << "Parsing error: " << line << std::endl;
        return;
    }
    CommandHandler cmd_handler(*this);
    cmd_handler.handleCommand(&message, client);
}

int Server::readClientData(int idx) {
    char buf[MAX_RECV];
    int client_fd = _pfds[idx].fd;

    int nbytes = recv(client_fd, buf, sizeof(buf), 0);

    if (nbytes < 0) {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
        std::cerr << "Error: recv(): " << strerror(errno) << std::endl;
        return -1;
    }
    else if (nbytes == 0) {
        return -1;
    }

    DEBUG_PRINT("[recv] ", client_fd, std::string(buf, nbytes));

    _clients[client_fd].getBuffer().append(buf, nbytes);
    if (_clients[client_fd].getBuffer().size() > MAX_BUFFER)
        return -1;

    size_t pos = 0;
    while ((pos = _clients[client_fd].getBuffer().find("\r\n")) != std::string::npos) {
        std::string line = _clients[client_fd].getBuffer().substr(0, pos + 2);
        _clients[client_fd].getBuffer().erase(0, pos + 2);
        if (line.size() > 512) //message cannot exceed 512 characters (incl. CRLF ("\r\n")) per RFC
            return -1;
        handleMessage(line, _clients[client_fd]);
    }
    return 0;
}

void Server::boot() {

    serverSocketSetup();

    while (!g_stop) {	

        if (poll(&_pfds[0], _pfds.size(), -1) == -1) {
            if (errno == EINTR)
                continue;
            throw std::runtime_error(std::string("poll() failed: ") + strerror(errno));
        }

        size_t i = 0;
        while (i < _pfds.size()) {
			if (_pfds[i].revents & POLLERR) {
                if (_pfds[i].fd != _server_fd)
                    disconnectClient(_clients[_pfds[i].fd], "POLLERR");
                continue;
            }
            else if (_pfds[i].revents & POLLHUP) {
                if (_pfds[i].fd != _server_fd)
                    disconnectClient(_clients[_pfds[i].fd], "POLLHUP");
                continue;
            }
            if (_pfds[i].revents & POLLIN) {
                if (_pfds[i].fd == _server_fd) {
                    acceptNewClient();
                }
                else if (_pfds[i].fd == STDIN_FILENO) {
					std::string input;
					std::getline(std::cin, input);

					if (input == "QUIT" || input == "quit" || input == "exit" || input == "EXIT") {						
                        for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ) {
                            Client &client = it->second;							
                            ++it;
							if (client.getFd() != -1)
                            	disconnectClient(client, "Server shutdown, goodbye!");
						}
						std::cout << "Server shutdown requested." << std::endl;
						g_stop = true;
					}
                }
                else {
                    if (readClientData(i) == -1) {
                        disconnectClient(_clients[_pfds[i].fd], "data cannot be read");
                        continue;
                    }
                }
            }
            i++;
        }
    }
}