#include "Client.hpp"

Client::Client() {}
Client::Client(int fd) : fd(fd), username("default_user"), nickname("default_nickname"), is_operator(false) {}
Client::Client(const Client& original) : fd(original.fd), username(original.username), nickname(original.nickname), is_operator(original.is_operator) {}
Client& Client::operator=(const Client& other) {
    if (this != &other) {
        fd = other.fd;
        username = other.username;
        nickname = other.nickname;
        is_operator = other.is_operator;
    }
    return *this;
}
Client::~Client() {}

int Client::getFd() const {
    return fd;
}