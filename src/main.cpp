#include "Server.hpp"

int main(int argc, char **argv) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    try {
        std::stringstream ss(argv[1]);
        int port;

        ss >> port;
        if (ss.fail() || !ss.eof() || port < 0 || port > 65535)
            throw std::runtime_error("invalid port number");

        Server server(port, argv[2]);
        server.boot();
    }
    catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
    return 0;
}