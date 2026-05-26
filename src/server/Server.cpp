#include "Server.hpp"

Server::Server() {}

Server::Server(int port, char *password) : port(port), password(password), server_fd(-1), creation_date("Mon May 15 2026") {
    //add checks for password?
}

Server::~Server() {
    if (server_fd != -1)
        close(server_fd);
    for (std::vector<pollfd>::iterator it = pfds.begin(); it != pfds.end(); ++it) {
        if (it->fd != -1)
            close(it->fd);
    }
}

std::string Server::getCreationDate() const {
    return creation_date;
}

std::string Server::getPassword() const {
    return password;
}

std::map<int, Client> Server::getClients() const {
    return clients;
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
    pfds.push_back(p);
}


void Server::removeClient(int idx) {
    int client_fd = pfds[idx].fd;
    clients.erase(client_fd);
    close(client_fd);
    pfds.erase(pfds.begin() + idx);
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

    int status = getaddrinfo(NULL, intToString(port).c_str(), &hints, &servinfo);
    if ((status != 0))
        throw std::runtime_error(gai_strerror(status));
    
    struct addrinfo *p;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server_fd == -1)
            continue;
        int yes = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket options: ") + strerror(errno));
        }
        //to detect dead clients (disconnected without sending a QUIT)
        if (setsockopt(server_fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket options: ") + strerror(errno));
        }
        //set socket fd in non-blocking mode
        if (fcntl(server_fd, F_SETFL, O_NONBLOCK) == -1) {
            freeaddrinfo(servinfo);
            throw std::runtime_error(std::string("failed to set socket to non-blocking: ") + strerror(errno));
        }
        if (bind(server_fd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        
        close(server_fd);
        server_fd = -1;
    }

    freeaddrinfo(servinfo);

    if (p == NULL)
        throw std::runtime_error(std::string("failed to bind socket: ") + strerror(errno));

    if (listen(server_fd, SOMAXCONN) == -1)
        throw std::runtime_error(std::string("failed to listen on socket: ") + strerror(errno));

    addToPoll(server_fd);
    std::cout << "IRC server listening on port " << port << std::endl;
}

void Server::acceptNewClient() {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
    if (new_fd == -1) {
        std::cerr << "Error: failed to accept client" << std::endl;
        return;
    }
    if (fcntl(new_fd, F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "Error: fcntl(): " << strerror(errno) << std::endl;
        close(new_fd);
        return;
    }
    addToPoll(new_fd);
    clients[new_fd] = Client(new_fd);
    clients[new_fd].setServer(this);
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
    clients[new_fd].setHost(ipstr);
}



void Server::handleMessage(std::string& line, Client& client) {
    s_msg message;
    if (parser(line, message) == -1) {
        std::cerr << "Parsing error: " << line << std::endl;
        return;
    }
/*     std::cout << "Prefix: " << message.prefix << std::endl;
    std::cout << "Command: " << message.command << std::endl;
    std::cout << "Parameters: " << std::endl;
    for (size_t i = 0; i < message.parameters.size(); i++) {
        std::cout << i << ": " << message.parameters[i] << std::endl;
    } */
    CommandHandler cmd_handler(*this);
    cmd_handler.handleCommand(&message, client);
    /*
    //broadcasting to all clients.
    for (size_t i = 0; i < pfds.size(); i++) {
        int dest_fd = pfds[i].fd;
        if (dest_fd != pfds[idx].fd && dest_fd != server_fd) {
            if (send(dest_fd, buf, nbytes, 0) == -1)
                std::cerr << "Error: send(): " << strerror(errno) << std::endl;
        }
    } */
}

int Server::readClientData(int idx) {
    char buf[MAX_LENGTH];
    int client_fd = pfds[idx].fd;

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
        std::cout << "Client fd " << client_fd << " hung up" << std::endl;
        return -1;
    }

    //appending message to client buffer: must check that it does not violate the 512 char max!
    clients[client_fd].getBuffer().append(buf, nbytes);
    size_t pos = 0;
    while ((pos = clients[client_fd].getBuffer().find("\r\n")) != std::string::npos) {
        std::string line = clients[client_fd].getBuffer().substr(0, pos + 2);
        handleMessage(line, clients[client_fd]);
        clients[client_fd].getBuffer().erase(0, pos + 2);
    }

    return 0;
}


void Server::boot() {

    serverSocketSetup();

    while (!g_stop) {

        if (poll(&pfds[0], pfds.size(), -1) == -1) {
            if (errno == EINTR)
                continue;
            throw std::runtime_error(std::string("poll() failed: ") + strerror(errno));
        }

        size_t i = 0;
        while (i < pfds.size()) {
            if (pfds[i].revents & POLLERR) {
                std::cerr << "POLLERR on fd " << pfds[i].fd << std::endl;
                removeClient(i);
                continue;
            }
            else if (pfds[i].revents & POLLHUP) {
                removeClient(i);
                continue;
            }
            if (pfds[i].revents & POLLIN) {
                if (pfds[i].fd == server_fd) {
                    acceptNewClient();
                }
                else {
                    if (readClientData(i) == -1) {
                        removeClient(i);
                        continue;
                    }
                }
            }
            i++;
        }
    }
}