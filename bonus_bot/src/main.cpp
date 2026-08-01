#include <iostream>
#include <signal.h>
#include <sstream>
#include <string.h>

#include <Bot.hpp>
#include <vector>

#define BW_PATH "texts/bad.txt"

volatile sig_atomic_t g_stop = 0;

void signalHandler(int signum) {
    (void)signum;
    g_stop = 1;
}

void usage( const char* name) {
    std::cerr << "Usage: " << name << " <net> <port> <passw> optional:<chan1,chan2,..>\n";
    std::cerr << "\n\t-net:       ipaddress/hostname of the irc server\n";
    std::cerr <<   "\t-port:      port of the irc server\n";
    std::cerr <<   "\t-passw:     password of the irc server\n";
    std::cerr <<   "\t-optional:  comma seperated list of channels to connect to\n";
    std::cerr << "\nwarning: put channel list into quotes for it to be recognized" << std::endl;
}

int main(int argc, char **argv) {

    if (argc != 4 && argc != 5) {
        usage(argv[0]);
        return 1;
    }

    try {

        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        signal(SIGPIPE, SIG_IGN);

        std::vector<std::string> channels;
        if (argc == 5) {
            if (strlen(argv[4]) < 2) {
                std::cerr << "not a valid channel name: " << argv[4] << std::endl;
                return 1;
            }

            std::stringstream ss(argv[4]);
            std::string buff;
            while(getline(ss, buff, ',')) {
                if (buff.length() > 0 && buff.at(0) != '&' && buff.at(0) != '#') {
                    std::cerr << "channels need to start with '#' or '&': e.g. '#channel'" << std::endl;
                    return 1;
                }
                channels.push_back(buff);
            }
        }

        Bot bot;
        bot.loadBadWords(BW_PATH);
        bot.connect(argv[1], argv[2]); // -> no check, throws
        bot.login(argv[3]);
        for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it) {
            bot.join(*it);
        }
        return (bot.run());
    }
    catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
}
