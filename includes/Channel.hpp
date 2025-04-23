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
};

#endif
