#ifndef OPERATOR_HPP
#define OPERATOR_HPP

#include "Client.hpp"

class Operator: public Client {
    public:
        Operator();
        Operator(int fd);
        Operator(const Operator& original);
        Operator& operator=(const Operator& other);
        ~Operator();
    
    private:

};

#endif