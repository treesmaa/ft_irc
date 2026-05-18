#include "Client.hpp"

Client::Client() {}
Client::Client(int fd) : fd(fd), username("default_user"), nickname("default_nickname"), registered(false) {}
Client::Client(const Client& original) : fd(original.fd), username(original.username), nickname(original.nickname), registered(original.registered) {}
Client& Client::operator=(const Client& other) {
    if (this != &other) {
        fd = other.fd;
        username = other.username;
        nickname = other.nickname;
        registered = other.registered;
    }
    return *this;
}
Client::~Client() {}

int Client::getFd() const {
    return fd;
}

bool Client::getRegistrationStatus() const {
    return registered;
}

void Client::setUsername(const std::string& name) {
    username = name;
}

void Client::setNickname(const std::string& name) {
    nickname = name;
}
void Client::registerClient() {
    registered = true;
}
