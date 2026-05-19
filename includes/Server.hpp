#ifndef SERVER_HPP
#define SERVER_HPP

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/poll.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>
#include <map>

#include "Client.hpp"
#include "Signal.hpp"
#include "CommandParser.hpp"
#include "CommandHandler.hpp"

class Server {
    public:
        Server(int port, char *password);
        ~Server();

        void boot(void);
        void serverSocketSetup(void);
        void addToPoll(int fd);
        void removeClient(int idx);
        void printNewClient(struct sockaddr_storage client_addr) const;
        void acceptNewClient();
        int readClientData(int idx);
        int handleRegistration(int client_fd);

    private:
        int                         port;
        std::string                 password;
        int                         server_fd;
        std::vector<struct pollfd>  pfds;
        std::map<int, Client>       clients;
        //not copiable
        Server(const Server& original);
        Server& operator=(const Server& other);
        //cannot be default created
        Server();
};

#endif