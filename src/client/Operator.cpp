#include "Operator.hpp"

Operator::Operator(): Client() {}
Operator::Operator(int fd): Client(fd) {}
Operator::Operator(const Operator& original): Client(original) {}
Operator& Operator::operator=(const Operator& other) {
    if (this != &other) {
        Client::operator=(other);
    }
    return *this;
}
Operator::~Operator() {}