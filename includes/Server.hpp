#ifndef SERVER_HPP
#define SERVER_HPP

#include <stdexcept>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>

class Server {
    public:
        Server();
        Server(char *port, char *password);
        Server(const Server& original);
        Server& operator=(const Server& other);
        ~Server();

        int start(void);
    private:
        std::string _port;
        std::string _password;
};

#endif