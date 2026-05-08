#ifndef SERVER_HPP
#define SERVER_HPP

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>

class Server {
    public:
        Server();
        Server(unsigned int port, char *password);
        Server(const Server& original);
        Server& operator=(const Server& other);
        ~Server();

        void boot(void);

    private:
        unsigned int        _port;
        std::string         _password;
        int                 _sockfd;
};

#endif