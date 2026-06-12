#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>
#include <stdbool.h>

class Client {
    public:
        Client();
        Client(int fd, const std::string& hostname);
        Client(const Client& original);
        Client& operator=(const Client& other);
        ~Client();
		//getters
        int								getFd() const;
        std::string 					getNickname() const;
        std::string 					getUsername() const;
        std::string 					getPassword() const;
        std::string 					getHost() const;
        bool							isRegistered() const;
        std::string&					getBuffer();
		std::set<std::string>&			getChannels();
		//setters
        void setUsername(const std::string& name);
        void setNickname(const std::string& name);
        void setPassword(const std::string& pass);
        void setHost(const std::string& host);
        void registerClient();

    private:
        int         				_fd;
        std::string 				_username;
        std::string 				_nickname;
        std::string 				_password;
        std::string 				_hostname;
        bool        				_registered;
        std::string 				_buf;
		std::set<std::string>		_channels;
};

#endif