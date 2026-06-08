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
#include <iomanip>

#include "Client.hpp"
#include "Channel.hpp"
#include "Signal.hpp"
#include "CommandParser.hpp"
#include "CommandHandler.hpp"

#define MAX_LENGTH 512

class Server {
    public:
        Server(int port, char *password);
        ~Server();
	
		//getters
        std::string				    getPassword() const;
        std::map<int, Client>	    getClients() const;
        std::map<std::string, Channel>& getChannels();
        Client*                     getClientByNickname(const std::string& nickname);
        std::string				    getCreationDate() const;


        void	boot(void);
        void	serverSocketSetup(void);
        void	addToPoll(int fd);
		void	disconnectClient(Client& client, std::string reason);
        void	removeClient(int fd);
		void	removePollFd(int client_fd);
        void    sendToClient(int fd, const std::string& message);
        void    broadcastToChannel(const std::string& channel_name, const std::string& message, int except_fd);
        void    removeClientFromChannels(int fd);
        
        //void printNewClient(struct sockaddr_storage client_addr) const;
        void	acceptNewClient();
        int		readClientData(int idx);
        void	handleMessage(std::string& line, Client& client);

    private:
        int                         _port;
        std::string                 _password;
        int                         _server_fd;
        std::vector<struct pollfd>  _pfds;
        std::map<int, Client>       _clients;
        std::map<std::string, Channel> _channels;
        std::string                 _creation_date;
        //not copiable
        Server(const Server& original);
        Server& operator=(const Server& other);
        //cannot be default created
        Server();
};

#endif