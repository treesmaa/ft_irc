#ifndef CLIENT_HPP
#define CLIENT_HPP

class Client {
    public:
        Client(int fd);
        Client(const Client& original);
        Client& operator=(const Client& other);
        ~Client();

        int getFd() const;
    private:
        int fd;

        Client();

};

#endif