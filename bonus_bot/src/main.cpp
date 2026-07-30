#include <iostream>
#include <signal.h>

#include <Bot.hpp>

volatile sig_atomic_t g_stop = 0;

void signalHandler(int signum) {
    (void)signum;
    g_stop = 1;
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

        Bot bot;
        bot.connect(argv[1], argv[2]); // -> no check, throws
        bot.login( argv[3] );
        return (bot.run());
    }
    catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
}
