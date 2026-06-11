#include "Client.hpp"

Client::Client()
    :   _fd(-1),
        _nickname("*"),
        _registered(false) {}

Client::Client(int fd, const std::string& hostname)
	:	_fd(fd),
		_nickname("*"),
		_hostname(hostname),
		_registered(false) {}

Client::Client(const Client& original)
	:	_fd(original._fd),
		_username(original._username),
		_nickname(original._nickname),
		_password(original._password),
		_hostname(original._hostname),
		_registered(original._registered),
		_buf(original._buf) {}

Client& Client::operator=(const Client& other) {
    if (this != &other) {
        _fd = other._fd;
        _username = other._username;
        _nickname = other._nickname;
        _password = other._password;
        _hostname = other._hostname;
        _registered = other._registered;
        _buf = other._buf;
    }
    return *this;
}
Client::~Client() {}

int Client::getFd() const {
    return _fd;
}

std::string Client::getNickname() const {
    return _nickname;
}

std::string Client::getUsername() const {
    return _username;
}

std::string Client::getPassword() const {
    return _password;
}

std::string Client::getHost() const {
    return _hostname;
}
bool Client::isRegistered() const {
    return _registered;
}

std::string& Client::getBuffer() {
    return _buf;
}

std::set<std::string>& Client::getChannels() {
    return _channels;
}

void Client::setUsername(const std::string& name) {
    _username = name;
}

void Client::setNickname(const std::string& name) {
    _nickname = name;
}

void Client::setPassword(const std::string& pass) {
    _password = pass;
}

void Client::setHost(const std::string& host) {
    _hostname = host;
}

void Client::registerClient() {
    _registered = true;
}
