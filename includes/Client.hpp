#ifndef CLIENT_HPP
#define CLIENT_HPP

class Client {
    public:
        Client(int fd);
        ~Client();

        int getFd() const;
    private:
        int fd;

        Client();
        //not copiable!
        Client(const Client& original);
        Client& operator=(const Client& other);
};

#endif