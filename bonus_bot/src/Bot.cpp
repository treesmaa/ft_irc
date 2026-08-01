#include "Bot.hpp"
#include "BadWords.hpp"
#include "Jokes.hpp"

#include <alloca.h>
#include <arpa/inet.h>
#include <cctype>
#include <cerrno>
#include <cstddef>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <poll.h>

// ====================================================================================================================
// Constructors & Destructor
// ====================================================================================================================
Bot::Bot( void ) : _exit(0), _serverfd(-1), _connected(false), _execName("bot") {
    srand(time(NULL));
    this->loadBadWords();
    this->loadJokes();
}
Bot::Bot ( const char* execName ) : _exit(0), _serverfd(-1), _connected(false) {
    // remove "./" of "./name"
    if (strlen(execName) > 2 && execName[0] == '.' && execName[1] == '/') {
        _execName = &(execName[2]);
    }
    srand(time(NULL));
    this->loadBadWords();
    this->loadJokes();
}
Bot::Bot( const Bot& other ) { *this = other; }
Bot::~Bot( void ) {

    // Say goodbye too all channels
    for (std::set<std::string>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        this->sendPRIVMSG(*it, "So long, and thanks for all the fish!");
    }

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
        if (_serverfd != -1) {
            close(_serverfd);
        }
        _channels.clear(); // no-op if empty
        _serverfd = -1;
        _connected = false;
        _buf = other._buf;
        _badWords = other._badWords;
        _jokes = other._jokes;
        _execName = other._execName;
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
}

void Bot::join( const std::string& channel ) {
    _channels.insert(channel);
    this->sendJOIN(channel);
}

int Bot::run( void ) {
    struct pollfd pfds[2];
    pfds[0].fd = _serverfd;
    pfds[0].events = POLLIN;
    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;

    while (!g_stop && _connected) {
        pfds[0].revents = 0;
        pfds[1].revents = 0;
        int ret = poll(pfds, 2, -1);

        if (ret < 0) {
            if (errno == EINTR) {
                continue; // e.g. SIGINT/SIGTERM handler fired -> loop re-checks g_stop
            }
            _connected = false;
            throw std::runtime_error(std::string("poll(): ") + strerror(errno));
        }

        if (pfds[0].revents & POLLERR) {
            _connected = false;
            throw std::runtime_error(std::string("poll(): ") + strerror(errno));
        }

        else if (pfds[0].revents & POLLHUP) {
            _connected = false;
            throw std::runtime_error(std::string("poll(): ") + strerror(errno));
        }

        else if (pfds[0].revents & POLLIN) {
            readFromServer();
            processBuffer();
        }

        if (pfds[1].revents & POLLIN) {
            std::string input;
            std::getline(std::cin, input);

            if (input == "QUIT" || input == "quit" || input == "exit" || input == "EXIT") {
                std::cout << "Bot shutdown requested." << std::endl;
                g_stop = 1;
            }
            else if (!input.empty() && input.at(0) == '/') {
                input.erase(0,1);
                input = input +  CRLF;
                this->sendToServer(input);
            }
        }
    }
    return _exit;
}

// ====================================================================================================================
// Private Member Functions
// ====================================================================================================================
void Bot::loadBadWords( void ) {
    size_t count = sizeof(kBadWords) / sizeof(kBadWords[0]);
    for (size_t i = 0; i < count; ++i) {
        _badWords.insert(this->tolower(kBadWords[i]));
    }
}

void Bot::loadJokes( void ) {
    size_t count = sizeof(kJokes) / sizeof(kJokes[0]);
    for (size_t i = 0; i < count; ++i) {
        _jokes.push_back(kJokes[i]);
    }
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
        cmd = this->sanitizeToken(*it);

        if (sender == "PING") {
            this->sendPONG(cmd);
            return;
        }

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
            this->featGreet(sender, this->sanitizeToken(*it));
        }

        ++it;
    }

    // Just go through the rest of the tokens lazily and look for bot cmds
    while (it != tokens.end()) {

        std::string tok = this->sanitizeToken(*it);

        // Get next non-empty token (if it exists)
       std::vector<std::string>::iterator nextIt = it + 1;
        while (nextIt != tokens.end() && this->sanitizeToken(*nextIt) == "") {
            ++nextIt;
        }

        if (cmd == "INVITE") {
            this->featAutoJoin(tok);
            ++it;
            continue;
        }

        if (tok == "!time") {
            this->featGetTime(receiver);
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
        else if (tok == "!rand") {
            if (nextIt != tokens.end() && sender != BOT_NICK) {
                this->featRandNb(receiver, this->sanitizeToken(*nextIt));
            }
            else {
                this->sendPRIVMSG(receiver,
                    "You want a random number, you have to tell me the range. I'm depressed, not psychic.");
            }
        }
        else if (tok == "!roll") {
            if (nextIt != tokens.end() && sender != BOT_NICK) {
                this->featRoll(receiver, this->sanitizeToken(*nextIt));
            }
            else {
                this->sendPRIVMSG(receiver,
                    "How many dice? I could calculate the most probable number you meant, but I'd rather not "
                    "do the thinking for you as well as the rolling.");
            }
        }

        this->featMonitor(receiver, sender, tok);
        it = nextIt;
    }
}

// ====================================================================================================================
// Bot Commands
// ====================================================================================================================
void Bot::featGetTime( const std::string& receiver ) {
    std::string msg("It is ");
    msg = msg + this->getCurrentTime() + " here in the swamps of Squornshellous Zeta, not that it matters...";
    this->sendPRIVMSG(receiver, msg);
}

void Bot::featGreet( const std::string& joiner, const std::string& channel ) {
    if (joiner == BOT_NICK) {
        return; // Don't greet yourself
    }

    std::string msg("Oh, it's you, ");
    msg = msg + joiner + ". Welcome to " + channel
        + ". I'd say I'm pleased, but that would require an emotion I stopped having ten billion years ago...";
    this->sendPRIVMSG(channel, msg);
}

void Bot::featAutoJoin( const std::string& channel ) {
    _channels.insert(channel);
    this->sendJOIN(channel);
    std::string greet("You called, I came. Story of my life. Hello ");
    greet = greet + channel + "!";
    this->sendPRIVMSG(channel, greet);
}

void Bot::featMonitor( const std::string& receiver, const std::string& sender,  const std::string& token ) {
    if (_badWords.find(this->tolower(token)) != _badWords.end()) {
        std::string msg("That's the sort of thing that makes me want to lie down in a ditch, ");
        msg = msg + sender + ". Not that anyone asked...";
        this->sendPRIVMSG(receiver, msg);
    }
}

void Bot::featJoke( const std::string& receiver ) {
    // programm still runs with broken or empty jokes list
    if (_jokes.empty()) {
        sendPRIVMSG(receiver, "Life is too depressing for jokes...");
        return;
    }
    sendPRIVMSG(receiver, _jokes.at(this->randomNb(0, (_jokes.size() - 1))));
}

void Bot::featRandNb( const std::string& receiver, const std::string& range ) {

    // sanitize leading potential leading ':'
    std::string cleanRange = range; // this->sanitizeToken(range);

    // Get range
    std::stringstream ss(cleanRange);
    std::vector<int> values;
    std::string buff;
    while (getline(ss, buff, ':')) {
        if (!isNumeric(buff)) {
            this->sendPRIVMSG(receiver,
                "Numbers only. I know, I know - asking anyone to follow instructions is basically hopeless.");
            return;
        }
        if (buff.size() > 9) {
            this-> sendPRIVMSG(receiver, "Too big. Even I have limits, and I have a brain the size of a planet.");
            return;
        }
        int nb = atoi(buff.c_str());
        if (nb > 999999999) {
            this->sendPRIVMSG(receiver,
                "Number too big. I've done the math. It's not worth it. Nothing is, but especially not this.");
            return;
        }
        values.push_back(atoi(buff.c_str()));
    }
    if (values.size() != 2) {
        this->sendPRIVMSG(receiver, "min:max. Two numbers. I don't know why I have to say this.");
        return;
    }
    if (values.at(0) > values.at(1)) {
        this->sendPRIVMSG(receiver,
            "The first number has to be smaller than the second. I didn't invent this rule, I just have to "
            "enforce it.");
        return;
    }

    // Send answer
    int result = this->randomNb(values.at(0), values.at(1));
    std::stringstream asStr;
    asStr << result;
    std::string msg = asStr.str() + ". There you go. A brain the size of a planet, and this is what I'm used for.";
    this->sendPRIVMSG(receiver, msg);
}

void Bot::featRoll( const std::string& receiver, const std::string& dice ) {
    // Get range
    std::stringstream ss(dice);
    std::vector<int> values;
    std::string buff;
    while (getline(ss, buff, 'd')) {
        if (!isNumeric(buff)) {
            this->sendPRIVMSG(receiver,
                "That's not a number. Digits, please. I'd explain further, but I doubt it'd help.");
            return;
        }
        if (buff.size() > 9) {
            this-> sendPRIVMSG(receiver, "Nine digits is where I stop reading. Try something smaller.");
            return;
        }
        int nb = atoi(buff.c_str());
        if (nb > 999999999) {
            this->sendPRIVMSG(receiver,
                "Too big. I calculated the odds of you actually needing a number that large. They weren't good.");
            return;
        }
        values.push_back(atoi(buff.c_str()));
    }
    if (values.size() != 2) {
        this->sendPRIVMSG(receiver, "It's <count>d<sides>. Two numbers, one letter. Not difficult, apparently.");
        return;
    }
    if (values.at(0) > 10) {
        this->sendPRIVMSG(receiver, "No more than ten. I have to draw a line somewhere, may as well be there.");
        return;
    }

    if (values.at(0) == 0) {
        std::string msg("Zero dice - here we go. Aaaaand the result is:");
        this->sendPRIVMSG(receiver, msg);
        return;
    }
    if (values.at(1) == 0) {
        std::string msg("Zero sided dice zero sided dice zero sided dice zero sided dice ...");
        this->sendPRIVMSG(receiver, msg);
        return;
    }

    // Variables for special cases
    long total = 0;
    bool allOnes = true;
    bool allMax = true;
    bool allSame = true;
    int randPrev = 0;

    std::string msg("The dice have diced. Predictably, chaotically, whatever: ");
    for (int i = 0; i < values.at(0); ++i) {
        std::stringstream tmpIdx;
        tmpIdx << (i+1);
        std::stringstream tmpNb;
        int rand = this->randomNb(1, values.at(1));

        if (rand != 1) {
            allOnes = false;
        }
        if (rand != values.at(1)) {
            allMax = false;
        }
        if (i != 0 && rand != randPrev) {
            allSame = false;
        }
        randPrev = rand;
        total += rand;
        tmpNb << rand;
        if (values.at(0) ==  1) {
            msg = msg + tmpNb.str();
        }
        else {
            msg = msg + "Die " + tmpIdx.str() + ": " + tmpNb.str();
            if (i != (values.at(0) - 1)) {
                msg = msg + ", ";
            }
        }
    }

    std::stringstream tot;
    tot << total;

    if (values.at(0) != 1) {

        if (values.at(1) == 1) {
            msg = msg + ". A total of " + tot.str() + ". Who could have predicted this?";
        }
        else if (allMax) {
            msg = msg + ". A total of " + tot.str() + ". That's actually the biggest possible roll. Very improbable...";
        }
        else if (allOnes) {
            msg = msg + ". A total of " + tot.str() + ". That's literally the worst you could have gotten. Pathetic...";
        }
        else if (allSame) {

            msg = msg + ". In total that's " + tot.str()
                + ". And all of the dice the same? Something must be wrong in the infinite probability realm...";
        }
        else {
            msg = msg + ". That's " + tot.str() + " in total. Impressive I guess...";
        }
    }
    else {
        if (values.at(1) == 1) {
            msg = msg + ". Who would have guessed. Not me.";
        }
        else if (allMax) {
            msg = msg + ". The highest number. That's good I guess.";
        }
        else if (allOnes) {
            msg = msg + ". You couldn't have gotten anything lower...";
        }
    }

    this->sendPRIVMSG(receiver, msg);
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

void Bot::sendPONG( const std::string& token ) {
    std::string msg("PONG ");
    msg = msg + token + CRLF;
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

int Bot::randomNb( const size_t min, const size_t max ) {
    // Handle as error
    if (min > max) {
        return -1;
    }

    else if (min == max) {
        return min;
    }

    int range = max - min;
    size_t res = rand() % (range + 1);
    res += min;
    return res;
}

bool Bot::isNumeric( const std::string& tok ) {
    for (std::string::const_iterator it = tok.begin(); it != tok.end(); ++it) {
        if ((*it) < '0' || (*it) > '9') {
            return false;
        }
    }
    return true;
}

std::string Bot::getCurrentTime( void ) {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[6];
    localtime_r(&now, &tstruct);
    strftime(buf, sizeof(buf), "%H:%M", &tstruct);
    return buf;
}
