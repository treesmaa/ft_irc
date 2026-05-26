#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <stdbool.h>

class Server;

class Client {
    public:
        Client();
        Client(int fd);
        Client(const Client& original);
        Client& operator=(const Client& other);
        ~Client();
		//getters
        int				getFd() const;
        std::string 	getNickname() const;
        std::string 	getUsername() const;
        std::string 	getPassword() const;
        std::string 	getHost() const;
        bool			isRegistered() const;
        std::string&	getBuffer();
        Server* 		getServer() const;
		//setters
        void setServer(Server *serv);
        void setUsername(const std::string& name);
        void setNickname(const std::string& name);
        void setPassword(const std::string& pass);
        void setHost(const std::string& host);
        void registerClient();

    private:
        int         fd;
        std::string username;
        std::string nickname;
        std::string password;
        std::string hostname;
        bool        registered;
        std::string buf;
        Server      *server;
        /*Nickname: max 9 chars, must be unique
        See the protocol grammar rules
        for what may and may not be used
        in a nickname. In addition to the nickname, all servers must have the
   following information about all clients: the real name of the host
   that the client is running on, the username of the client on that
   host, and the server to which the client is connected.*/

};

#endif