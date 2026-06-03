#include "Server.hpp"

Server::Server() {}

Server::Server(int port, char *password)
    :   _port(port),
        _password(password),
        _server_fd(-1),
        _creation_date("Mon May 15 2026") {}

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
    removePollFd(fd);
    _clients.erase(fd);
    close(fd);
}

/* void Server::printNewClient(struct sockaddr_storage client_addr) const {
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
        if (!printable_addr)
            std::cerr << "Error: inet_ntop() failed: " << strerror(errno) << std::endl;
        else
            std::cout << "New connection from " << ipstr << std::endl;//can only be printed if success
} */

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
    std::cout << std::left << std::setw(14) << "[disconnect]" << " fd=" << client_fd << " host=" << _clients[client_fd].getHost() << " nickname=" << _clients[client_fd].getNickname() << " reason=" << reason << std::endl;
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
    char buf[MAX_LENGTH];
    int client_fd = _pfds[idx].fd;

    int nbytes = recv(client_fd, buf, sizeof(buf), 0);

    if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
        else if (errno == EINTR)
            return 0;
        std::cerr << "Error: recv(): " << strerror(errno) << std::endl;
        return -1;
    }
    else if (nbytes == 0) {
        return -1;
    }

    _clients[client_fd].getBuffer().append(buf, nbytes);
    size_t pos = 0;
    while ((pos = _clients[client_fd].getBuffer().find("\r\n")) != std::string::npos) {
        std::string line = _clients[client_fd].getBuffer().substr(0, pos + 2);
        _clients[client_fd].getBuffer().erase(0, pos + 2);
        if (line.size() > 512)//message cannot exceed 512 characters (incl. CRLF ("\r\n")) per RFC
            line = line.substr(0, 510) + CRLF;
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
                disconnectClient(_clients[_pfds[i].fd], "POLLERR");
                continue;
            }
            else if (_pfds[i].revents & POLLHUP) {
                disconnectClient(_clients[_pfds[i].fd], "POLLHUP");
                continue;
            }
            if (_pfds[i].revents & POLLIN) {
                if (_pfds[i].fd == _server_fd) {
                    acceptNewClient();
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