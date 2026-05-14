#include "Client.hpp"

Client::Client() {}
Client::Client(int fd) : fd(fd) {}
Client::Client(const Client& original) : fd(original.fd), username(original.username), nickname(original.nickname) {}
Client& Client::operator=(const Client& other) {
    if (this != &other) {
        fd = other.fd;
        username = other.username;
        nickname = other.nickname;
    }
    return *this;
}
Client::~Client() {}

int Client::getFd() const {
    return fd;
}