#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <stdbool.h>

class Client {
    public:
        Client();
        Client(int fd, const std::string& hostname);
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
		//setters
        void setUsername(const std::string& name);
        void setNickname(const std::string& name);
        void setPassword(const std::string& pass);
        void setHost(const std::string& host);
        void registerClient();

    private:
        int         _fd;
        std::string _username;
        std::string _nickname;
        std::string _password;
        std::string _hostname;
        bool        _registered;
        std::string _buf;
        /*Nickname: max 9 chars, must be unique
        See the protocol grammar rules
        for what may and may not be used
        in a nickname. In addition to the nickname, all servers must have the
   following information about all clients: the real name of the host
   that the client is running on, the username of the client on that
   host, and the server to which the client is connected.*/

};

#endif