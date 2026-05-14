#include "Client.hpp"

Client::Client() {}
Client::Client(int fd) : fd(fd) {}
Client::Client(const Client& original) : fd(original.fd) {}
Client& Client::operator=(const Client& other) {
    if (this != &other) {
        fd = other.fd;
    }
    return *this;
}
Client::~Client() {}

int Client::getFd() const {
    return fd;
}