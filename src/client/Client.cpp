#include "Client.hpp"
#include "Server.hpp"

Client::Client() {}
Client::Client(int fd) : fd(fd), username(""), nickname("*"), password(""), registered(false){}
Client::Client(const Client& original) : fd(original.fd), username(original.username), nickname(original.nickname), registered(original.registered), server(original.server) {}
Client& Client::operator=(const Client& other) {
    if (this != &other) {
        fd = other.fd;
        username = other.username;
        nickname = other.nickname;
        registered = other.registered;
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

std::string Client::getNickname() const {
    return nickname;
}

std::string Client::getUsername() const {
    return username;
}

std::string Client::getPassword() const {
    return password;
}

std::string Client::getHost() const {
    return hostname;
}
bool Client::isRegistered() const {
    return registered;
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

void Client::setPassword(const std::string& pass) {
    password = pass;
}

void Client::setHost(const std::string& host) {
    hostname = host;
}

void Client::registerClient() {
    registered = true;
}
