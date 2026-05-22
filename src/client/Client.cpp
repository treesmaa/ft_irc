#include "Client.hpp"
#include "Server.hpp"

Client::Client() {}
Client::Client(int fd) : fd(fd), username("default_user"), nickname("default_nickname"), registered(false), auth(false) {}
Client::Client(const Client& original) : fd(original.fd), username(original.username), nickname(original.nickname), registered(original.registered), auth(original.auth), server(original.server) {}
Client& Client::operator=(const Client& other) {
    if (this != &other) {
        fd = other.fd;
        username = other.username;
        nickname = other.nickname;
        registered = other.registered;
        auth = other.auth;
        server = other.server;
    }
    return *this;
}
Client::~Client() {}

Server* Client::getServer() const {
    return server;
}

int Client::getFd() const {
    return fd;
}

bool Client::isRegistered() const {
    return registered;
}

bool Client::isAuthenticated() const {
    return auth;
}

std::string& Client::getBuffer() {
    return buf;
}

void Client::setServer(Server *serv) {
    server = serv;
}

void Client::setUsername(const std::string& name) {
    username = name;
}

void Client::setNickname(const std::string& name) {
    nickname = name;
}

void Client::authenticate() {
    auth = true;
}

void Client::registerClient() {
    registered = true;
}
