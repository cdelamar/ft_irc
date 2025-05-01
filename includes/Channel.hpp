#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <set>
#include <vector>
#include "Client.hpp"
#include "Server.hpp"

class Server;

class Channel
{
private:
    std::string _name;
    std::string _topic;
    std::map<int, Client*> _members;
    std::set<int> _operators;
    std::set<int> _invited;

    // pour MODE
    bool _inviteOnly;
    bool _topicRestricted;
    std::string _password;
    size_t _userLimit;
    bool _limitMode;

public:
    // Canonical Form
    Channel();
    Channel(const std::string &name);
    Channel(const Channel &other);
    Channel &operator=(const Channel &other);
    ~Channel();

    void addMember(Client &client);
    void removeMember(int clientFd);
    bool isMember(int clientFd) const;

    void promoteToOperator(int clientFd);
    void demoteFromOperator(int clientFd);
    bool isOperator(int clientFd) const;

    void broadcast(Server &server, const std::string &message, int exceptFd) const;
    void setTopic(const std::string &topic);

    size_t              memberCount() const;          // nombre de membres
    std::vector<int>    getMemberFds() const;         // liste des FDs

    const std::string &getTopic() const;
    const std::string &getName() const;

    // pour MODE, prochainement
    bool isInviteOnly() const;
    bool isTopicRestricted() const;
    bool hasPassword() const;
    const std::string &getPassword() const;
    bool hasUserLimit() const;
    size_t getUserLimit() const;

    void setInviteOnly(bool);
    void setTopicRestricted(bool);
    void setPassword(const std::string &);
    void setUserLimit(size_t);
    void clearPassword();
    void clearUserLimit();

    std::string strModes() const;

    void addInvite(int fd);
    bool isInvited(int fd) const;
    void removeInvite(int fd);
};

#endif
