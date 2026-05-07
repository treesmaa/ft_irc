#ifndef SERVER_HPP
#define SERVER_HPP

#include <stdexcept>
#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>

class Server {
    public:
        Server();
        Server(char *port, char *password);
        Server(const Server& original);
        Server& operator=(const Server& other);
        ~Server();

        void boot(void);
    private:
        char         		*_port;
        std::string         _password;
        int                 _sockfd;
        struct sockaddr_in	_addr;
};

#endif