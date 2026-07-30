#include "Bot.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#define IN_PROMPT  ">>>  "
#define OUT_PROMPT "<<<  "

// Constructors & Destructor
Bot::Bot( void ) : _exit(0), _serverfd(-1), _connected(false) {}
Bot::Bot( const Bot& other ) { *this = other; }
Bot::~Bot( void ) {
    if (_serverfd != -1) {
        close(_serverfd);
    }
}

// Operators
Bot& Bot::operator=( const Bot& other ) {
    if (this != &other) {
        _exit = other._exit;
        _connected = other._connected;
    }
    return (*this);
}

// General methods
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
    sendToServer("PASS " + password + CRLF);
    sendToServer("NICK " + std::string(BOT_NICK) + CRLF);
    sendToServer("USER " + std::string(BOT_USER) + " 0 * :Whatever" + CRLF);
    sendToServer("JOIN #test" + std::string(CRLF));
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

// ==============================================================================================
// Private Member Functions
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

// TODO: Remove here
#include <ctime>
#include <iostream>
#include <string>
#include <stdio.h>
#include <time.h>

// Get the current date/time. The format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about the date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
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
        if (msg.find("!time") != std::string::npos) {

            std::string answer("PRIVMSG #test :");
            std::string currTime(currentDateTime());
            std::string end(CRLF);

            sendToServer(answer + currTime + end);
        }
        // TODO: React to msg
    }
}
