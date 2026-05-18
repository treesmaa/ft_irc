#include "Server.hpp"

volatile sig_atomic_t g_stop = 0;

void signalHandler(int signum) {
    (void)signum;
    g_stop = 1;
}

int convertPort(char *str) {
    std::stringstream ss(str);
    int port;

    ss >> port;
    if (ss.fail() || !ss.eof() || port < 1024 || port > 65535)//ports that can be used without root privilege
        throw std::runtime_error("invalid port number");
    return port;
}

int main(int argc, char **argv) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    try {
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        signal(SIGPIPE, SIG_IGN);

        Server server(convertPort(argv[1]), argv[2]);
        server.boot();
    }
    catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
    return 0;
}