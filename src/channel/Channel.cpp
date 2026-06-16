#include "Channel.hpp"

Channel::Channel() {}

Channel::Channel(const std::string& name) : _name(name) {}

Channel::Channel(const Channel& original)
    : _name(original._name), _members(original._members) {}

Channel& Channel::operator=(const Channel& other) {
    if (this != &other) {
        _name = other._name;
        _members = other._members;
    }
    return *this;
}

Channel::~Channel() {}

const std::string& Channel::getName() const {
    return _name;
}

const std::set<int>& Channel::getMembers() const {
    return _members;
}

bool Channel::hasMember(int fd) const {
    return _members.find(fd) != _members.end();
}

bool Channel::isEmpty() const {
    return _members.empty();
}

void Channel::addMember(int fd) {
    _members.insert(fd);
}

void Channel::removeMember(int fd) {
    _members.erase(fd);
}

void Channel::addOperator(int fd) {
	_operators.insert(fd);
}

void Channel::removeOperator(int fd) {
	_operators.erase(fd);
}

bool Channel::isOperator(int fd) const {
	return _operators.find(fd) != _operators.end();
}
