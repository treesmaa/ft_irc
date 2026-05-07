#include "Server.hpp"

int main(int argc, char **argv) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    try {
        Server server(argv[1], argv[2]);
        server.boot();
    }
    catch(const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 2;
    }
    return 0;
}