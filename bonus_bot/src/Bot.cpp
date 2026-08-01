#include "Bot.hpp"

#include <alloca.h>
#include <arpa/inet.h>
#include <cctype>
#include <cerrno>
#include <cstddef>
#include <ctime>
#include <fstream>
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
Bot::Bot( void ) : _exit(0), _serverfd(-1), _connected(false), _execName("bot") {}
Bot::Bot ( const char* execName ) : _exit(0), _serverfd(-1), _connected(false) {
    // remove "./" of "./name"
    if (strlen(execName) > 2 && execName[0] == '.' && execName[1] == '/') {
        _execName = &(execName[2]);
    }
}
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
void Bot::loadBadWords( const char* path ) {
    std::ifstream file;
    file.open(path);
    if (!file.good()) {
        throw std::runtime_error(std::string(_execName) + std::string(": could not open badwords file: ") + strerror(errno));
    }

    std::string buff;
    while (getline(file, buff, '\n')) {
        _badWords.insert(this->tolower(buff));
    }

    file.close();
}

void Bot::loadJokes( const char* path ) {
    std::ifstream file;
    file.open(path);
    if (!file.good()) {
        std::cout << _execName << ": jokes could not be loaded: " << strerror(errno) << std::endl;
        return;
    }

    std::string buff;
    while (getline(file, buff, '\n')) {
        if (buff != "" && buff.at(0) != '#' && buff.length() < 510) {
            _jokes.push_back(this->tolower(buff));
        }
    }
    if (_jokes.empty()) {
        std::cout << _execName << ": joke file empty. no jokes have been loaded" << std::endl;
    }
    file.close();
}

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
         // std::cout << "tok: " << buffer << std::endl; // TODO: Remove - debug only
    }


    std::string sender;
    std::string cmd;
    std::string receiver;

    std::vector<std::string>::iterator it = tokens.begin();
    if (it != tokens.end()) {
        sender = this->sanitizeToken(*it);
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

        if (cmd == "JOIN") {
            this->featGreet(sender, *it);
        }

        ++it;
    }

    if (it != tokens.end()) {

        if (cmd == "INVITE") {
            this->featAutoJoin(*it);
            return;
        }

        ++it;
    }

    // Just go through the rest of the tokens lazily and look for bot cmds
    while (it != tokens.end()) {
        std::string tok = this->sanitizeToken(*it);

        if (tok == "!time") {
            sendPRIVMSG(receiver, "Its currently");
        }
        else if (tok == "!panic") {
            sendPRIVMSG(receiver, "don't");
        }
        else if (tok == "!answer") {
            sendPRIVMSG(receiver, "42");
        }
        else if (tok == "!joke") {
            this->featJoke(receiver);
        }

        this->featMonitor(receiver, sender, tok);
        ++it;
    }
}

// ====================================================================================================================
// Bot Commands
// ====================================================================================================================
void Bot::featGreet( const std::string& joiner, const std::string& channel ) {
    if (joiner == BOT_NICK) {
        return; // Don't greet yourself
    }
    std::string msg("Hey ");
    std::string chan = this->sanitizeToken(channel);
    msg = msg + joiner + ", welcome to the " + chan + " channel!";
    this->sendPRIVMSG(chan, msg);
}

void Bot::featAutoJoin( const std::string& channel ) {
    std::string chan = this->sanitizeToken(channel);
    this->sendJOIN(chan);
    std::string greet("Hello ");
    greet = greet + chan + ", i hope everyone is doing great today!";
    this->sendPRIVMSG(chan, greet);
}

void Bot::featMonitor( const std::string& receiver, const std::string& sender,  const std::string& token ) {
    if (_badWords.find(this->tolower(token)) != _badWords.end()) {
        std::string msg("We don't usually say such disgusting things around here ");
        msg = msg + sender + " ...";
        this->sendPRIVMSG(receiver, msg);
        // TODO: Add kick if possible
    }
}

void Bot::featJoke( const std::string& receiver ) {
    // programm still runs with broken or empty jokes list
    if (_jokes.empty()) {
        sendPRIVMSG(receiver, "Life is too depressing for jokes...");
        return;
    }

    sendPRIVMSG(receiver, _jokes.at(0));
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

// ====================================================================================================================
// Helper Functions
// ====================================================================================================================
std::string Bot::tolower( const std::string& token ) {
    std::string res = token;
    for (std::string::iterator it = res.begin(); it != res.end(); ++it) {
        (*it) = std::tolower(*it);
    }
    return res;
}

const char* Bot::checkPort(const char *str) {
    int asInt = std::atoi(str);
    if (asInt < 1024 || asInt > 65535)
        throw std::runtime_error("invalid port number");
    return str;
}

std::string Bot::sanitizeToken( const std::string& sender ) {
    std::string res = "";
    std::string::const_iterator it = sender.begin();
    if (sender.length() > 0 && sender.at(0) == ':') {
        ++it;
    }
    if (it != sender.end() && *it == '!') { // to still get bot cmds firing
        res.push_back(*it);
        ++it;
    }
    while (it != sender.end() && (*it) != ' '  && (*it) != '!'
                              && (*it) != '\r' && (*it) != '\n') {
        res.push_back(*it);
        ++it;
    }
    return res;
}
