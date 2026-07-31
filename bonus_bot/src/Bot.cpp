#include "Bot.hpp"

#include <alloca.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <vector>

// ====================================================================================================================
// Constructors & Destructor
// ====================================================================================================================
Bot::Bot( void ) : _exit(0), _serverfd(-1), _connected(false) {}
Bot::Bot( const Bot& other ) { *this = other; }
Bot::~Bot( void ) {
    if (_serverfd != -1) {
        close(_serverfd);
    }
}

// ====================================================================================================================
// Operator overloads
// ====================================================================================================================
Bot& Bot::operator=( const Bot& other ) {
    if (this != &other) {
        _exit = other._exit;
        _connected = other._connected;
    }
    return (*this);
}

// ====================================================================================================================
// Public methods
// ====================================================================================================================
void Bot::connect( const std::string& network, const std::string& port) {
    struct addrinfo hints, *res;

    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(network.c_str(), this->checkPort(port.c_str()), &hints, &res);
    if (status != 0) {
        throw std::runtime_error(gai_strerror(status));
    }

    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {

        // Try opening a socket
        _serverfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (_serverfd == -1) {
            continue;
        }

        // ::connect to not get naming conflict
        if (::connect(_serverfd, p->ai_addr, p->ai_addrlen) == -1) {
            // perror("client: connect");
            close(_serverfd);
            _serverfd = -1;
            continue;
        }

        // set socket fd in non-blocking mode
        if (fcntl(_serverfd, F_SETFL, O_NONBLOCK) == -1) {
            freeaddrinfo(res);
            close(_serverfd);
            _serverfd = -1;
            throw std::runtime_error(std::string("failed to set socket to non-blocking: ") + strerror(errno));
        }
        break;
    }

    freeaddrinfo(res);

    if (p == NULL)
        throw std::runtime_error(std::string("failed to connect: ") + strerror(errno));

    _connected = true;
}

void Bot::login( const std::string& password ) {
    sendPASS(password);
    sendNICK(BOT_NICK);
    sendUSER(BOT_USER, "0", "*", "Zaphod Beeblebot The First");
    // sendToServer("JOIN #test" + std::string(CRLF));
}

void Bot::join( const std::string& channel ) {
    this->sendJOIN(channel);
}

int Bot::run( void ) {
    while (!g_stop && _connected) {
        // TODO: add poll here instead
        readFromServer();  // -> Handles commands inside
        processBuffer();
        sleep(1); // TODO: remove once we use poll
    }
    return _exit;
}

// ====================================================================================================================
// Private Member Functions
// ====================================================================================================================
const char* Bot::checkPort(const char *str) {
    int asInt = std::atoi(str);
    if (asInt < 1024 || asInt > 65535)
        throw std::runtime_error("invalid port number");
    return str;
}

void Bot::sendToServer(const std::string& message) {
    if (_serverfd < 0)
        return;
    if (not _connected) {
        std::cerr << "Send error: not connected to any server" << std::endl;
    }

    std::cout << OUT_PROMPT << message;

    if (send(_serverfd, message.c_str(), message.size(), 0) == -1)
        std::cerr << "Send error" << std::endl;
}

void Bot::readFromServer( void ) {
    char buf[MAX_LENGTH];

    int nbytes = recv(_serverfd, buf, sizeof(buf), 0);

    if (nbytes < 0) {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        throw std::runtime_error(std::string("recv(): ") + strerror(errno));
    }
    else if (nbytes == 0) {
        throw std::runtime_error(std::string("recv(): ") + strerror(errno));
    }
    _buf.append(buf, nbytes);
}

std::string Bot::sanitize( const std::string& str ) {
    std::string clean;

    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        if (*it != '\n' && *it != '\r') {
            clean.push_back(*it);
        }
    }
    return clean;
}

std::string Bot::sanitizeSender( const std::string& sender ) {
    std::string res = "";
    std::string::const_iterator it = sender.begin();
    ++it;
    while (it != sender.end() && (*it) != ' '  && (*it) != '!'
                              && (*it) != '\r' && (*it) != '\n') {
        res.push_back(*it);
        ++it;
    }
    return res;
}

void Bot::processBuffer( void ) {
    size_t delimPos = 0;
    while ((delimPos = _buf.find("\r\n")) != std::string::npos) {

        std::string msg = _buf.substr(0, delimPos + 2);
        _buf.erase(0, delimPos + 2);

        if (msg.size() > 512) {
            msg = msg.substr(0, 510) + CRLF;
        }

        std::cout << IN_PROMPT << msg;
        this->processMessage(msg);
    }
}

void Bot::processMessage( const std::string& message ) {
    std::stringstream ss(message);
    std::string buffer;
    std::vector<std::string> tokens;

    while (getline(ss, buffer, ' ')) {
        tokens.push_back(buffer);
        std::cout << "tok: " << buffer << std::endl; // TODO: Remove - debug only
    }


    std::string sender;
    std::string cmd;
    std::string receiver;

    std::vector<std::string>::iterator it = tokens.begin();
    if (it != tokens.end()) {
        sender = this->sanitizeSender(*it);
        ++it;
    }

    if (it != tokens.end()) {
        cmd = *it;
        if (cmd == "464") {
            throw std::runtime_error(std::string("incorrect password"));
        }

        ++it;
    }

    if (it != tokens.end()) {
        receiver = *it;

        // if we are the receiver we send back to sender, else to same receiver aka channel
        if (receiver == BOT_NICK) {
            receiver = sender;
        }

        // bot_feat: greet after joining
        if (cmd == "JOIN") {
            std::string msg("Hey ");
            std::string chan = this->sanitizeSender(*it);
            msg = msg + sender + ", welcome to the " + chan + " channel!";
            this->sendPRIVMSG(chan, msg);
        }

        ++it;
    }

    // Handle invite on 4th token
    if (it != tokens.end()) {

        // bot_feat: join a channel when receiving an invite
        if (cmd == "INVITE" && ((*it).find("#") != std::string::npos || (*it).find("&") != std::string::npos )) {
            this->sendJOIN(*it);
            std::string greet("Hello ");
            std::string channel = this->sanitize(*it);
            greet = greet + channel + ", i hope everyone is doing great today!";
            sendPRIVMSG(channel, greet);
            return;
        }

        ++it;
    }

    // Just go through the rest of the tokens lazily and look for bot cmds
    while (it != tokens.end()) {
        std::string tok = this->sanitize(*it);

        // bot_feat: "!time" tells current time
        if (tok == "!time") {
            sendPRIVMSG(receiver, "Its currently");
        }

        ++it;
    }
}

// ====================================================================================================================
// Server Commands
// ====================================================================================================================
void Bot::sendPASS( const std::string& password ) {
    std::string msg("PASS ");
    msg = msg + password + CRLF;
    this->sendToServer(msg);
}

void Bot::sendNICK( const std::string& nickname ) {
    std::string msg("NICK ");
    msg = msg + nickname + CRLF;
    this->sendToServer(msg);
}

void Bot::sendUSER( const std::string& username, const std::string& mode,
                        const std::string& unused, const std::string& in_msg ) {
    std::string msg("USER ");
    msg = msg + username + " " + mode + " " + unused + " :" + in_msg + CRLF;
    this->sendToServer(msg);
}

void Bot::sendJOIN( const std::string& channel ) {
    std::string msg("JOIN ");
    msg = msg + channel + CRLF;
    this->sendToServer(msg);
}

void Bot::sendPRIVMSG( const std::string& receiver, const std::string& in_msg ) {
    std::string msg("PRIVMSG ");
    msg = msg + receiver + " :" + in_msg + CRLF;
    this->sendToServer(msg);
}
