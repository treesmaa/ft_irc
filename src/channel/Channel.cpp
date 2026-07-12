#include "Channel.hpp"

Channel::Channel() {}

Channel::Channel(const std::string& name) : _name(name), _inviteOnly(false), _hasPassword(false), _password(""),_hasRestrictedTopic(false), _topic(""), _memberLimit(-1) {}

Channel::Channel(const Channel& original)
    : _name(original._name), _members(original._members), _operators(original._operators), _invitedUsers(original._invitedUsers), _inviteOnly(original._inviteOnly), _hasPassword(original._hasPassword), _password(original._password), _hasRestrictedTopic(original._hasRestrictedTopic), _topic(original._topic), _memberLimit(original._memberLimit) {}

Channel& Channel::operator=(const Channel& other) {
    if (this != &other) {
        _name = other._name;
        _members = other._members;
		_operators = other._operators;
		_invitedUsers = other._invitedUsers;
        _inviteOnly = other._inviteOnly;
        _hasPassword = other._hasPassword;
        _password = other._password;
		_hasRestrictedTopic = other._hasRestrictedTopic;
        _topic = other._topic;
        _memberLimit = other._memberLimit;
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

bool Channel::isInvited(int fd) const {
	return _invitedUsers.find(fd) != _invitedUsers.end();
}

void Channel::addInvitedUser(int fd) {
	_invitedUsers.insert(fd);
}

void Channel::removeInvitedUser(int fd) {
	_invitedUsers.erase(fd);
}

void Channel::setInviteOnly(bool inviteOnly) {
	_inviteOnly = inviteOnly;
}

bool Channel::isInviteOnly() const {
	return _inviteOnly;
}

bool Channel::hasPassword() const {
	return _hasPassword;
}

void Channel::setPassword(const std::string& password) {
	_password = password;
	_hasPassword = true;
}

void Channel::removePassword() {
	_password = "";
	_hasPassword = false;
}

std::string Channel::getPassword() const {
	return _password;
}

bool Channel::hasRestrictedTopic() const {
	return _hasRestrictedTopic;
}

void Channel::setRestrictedTopic(bool restricted) {
	_hasRestrictedTopic = restricted;
}

void Channel::setTopic(const std::string& topic) {
	_topic = topic;
}

const std::string& Channel::getTopic() const {
	return _topic;
}

int Channel::getMemberLimit() const {
	return _memberLimit;
}

void Channel::setMemberLimit(int limit) {
	_memberLimit = limit;
}

bool Channel::hasLimit() const {
	return _memberLimit > 0;
}