#include "Client.hpp"

Client::Client() {}
Client::Client(int fd) : fd(fd) {}
Client::~Client() {}

int Client::getFd() const {
    return fd;
}