#ifndef CLIENT_HPP
#define CLIENT_HPP

class Client {
    public:
        Client();
        ~Client();
    private:
        int fd;

        //not copiable!
        Client(const Client& original);
        Client& operator=(const Client& other);
};

#endif