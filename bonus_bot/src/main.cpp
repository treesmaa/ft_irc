#include <iostream>
#include <signal.h>
#include <sstream>

#include <Bot.hpp>

volatile sig_atomic_t g_stop = 0;

void signalHandler(int signum) {
    (void)signum;
    g_stop = 1;
}

int convertPort(char *str) {
    std::stringstream ss(str);
    int port;

    ss >> port;
    // ports that can be used without root privilege
    if (ss.fail() || !ss.eof() || port < 1024 || port > 65535)
        throw std::runtime_error("invalid port number");
    return port;
}

int main(int argc, char **argv) {

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <network> <port> <password>" << std::endl;
        return 1;
    }

    try {
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        signal(SIGPIPE, SIG_IGN);

        Bot bot(argv[1], convertPort(argv[2]), argv[3]);
        bot.monitor(); // -> Main loop of the bot
        return (bot.exit());
    }
    catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
}
