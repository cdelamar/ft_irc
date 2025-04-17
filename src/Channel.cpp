#include "Channel.hpp"
#include <iostream>

Channel::Channel()
    : _name(""), _topic(""), _members(), _operators()
{
}

Channel::Channel(const std::string &name)
    : _name(name), _topic(""), _members(), _operators()
{
}

Channel::Channel(const Channel &other)
    : _name(other._name),
      _topic(other._topic),
      _members(other._members),
      _operators(other._operators)
{
}

Channel &Channel::operator=(const Channel &other)
{
    if (this != &other)
    {
        _name = other._name;
        _topic = other._topic;
        _members = other._members;
        _operators = other._operators;
    }
    return *this;
}

Channel::~Channel() {}


void Channel::addMember(Client &client)
{
    _members[client.getFd()] = &client;
}

void Channel::removeMember(int clientFd)
{
    _members.erase(clientFd);
    _operators.erase(clientFd);
}

bool Channel::isMember(int clientFd) const
{
    return (_members.find(clientFd) != _members.end());
}

void Channel::promoteToOperator(int clientFd)
{
    if (isMember(clientFd))
        _operators.insert(clientFd);
}

void Channel::demoteFromOperator(int clientFd)
{
    _operators.erase(clientFd);
}

bool Channel::isOperator(int clientFd) const
{
    return (_operators.find(clientFd) != _operators.end());
}

void Channel::broadcast(const std::string &message, int exceptFd) const
{
    std::map<int, Client*>::const_iterator it = _members.begin();
    while (it != _members.end())
    {
        if (it->first != exceptFd)
        {
            it->second->appendToBuffer(message + "\r\n");
        }
        ++it;
    }
}

void Channel::setTopic(const std::string &topic)
{
    _topic = topic;
}

size_t Channel::memberCount() const
{
    return _members.size();
}

std::vector<int> Channel::getMemberFds() const
{
    std::vector<int> v;
    v.reserve(_members.size());
    for (std::map<int, Client*>::const_iterator it = _members.begin();
         it != _members.end(); ++it)
        v.push_back(it->first);
    return v;
}

const std::string &Channel::getTopic() const
{
    return _topic;
}

const std::string &Channel::getName() const
{
    return _name;
}
