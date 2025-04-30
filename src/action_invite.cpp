#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

static Client* nickFinder(Server &srv, const std::string &nick)
{
    const std::map<int, Client> &clients = srv.getClients();
    std::map<int, Client>::const_iterator it = clients.begin();

    while (it != clients.end())
	{
        if (it->second.getNickname() == nick)
            return &(srv.getClient(it->first));
        ++it;
    }

    return 0;
}

static bool already_in(Channel &c, int fd)
{
    return c.isMember(fd);
}

static void invite(Server &srv, Client &src, Client &dest, Channel &channel) {
    channel.addInvite(dest.getFd());

    std::string ch = channel.getName();
    std::string n1 = src.getNickname();
    std::string n2 = dest.getNickname();

    srv.sendToClient(src.getFd(), "341 " + n2 + " " + ch);
    srv.sendToClient(dest.getFd(), ":" + n1 + " INVITE " + n2 + " :" + ch);
}

void handleInvite(Server &server, int clientFd, const Command &cmd)
{
    if (cmd.params.size() < 2)
        return server.sendToClient(clientFd, "461 INVITE :Need more parameters");

    std::string nick = cmd.params[0];
    std::string chan = cmd.params[1];

    if (!server.channelExists(chan))
        return server.sendToClient(clientFd, "403 " + chan + " :No such channel");

    Channel &c = server.getChannel(chan);
    if (!c.isMember(clientFd))
        return server.sendToClient(clientFd, "442 " + chan + " :You're not on that channel");

    if (!c.isOperator(clientFd))
        return server.sendToClient(clientFd, "482 " + chan + " :You're not channel operator");

    Client *target = nickFinder(server, nick);
    if (!target)
        return server.sendToClient(clientFd, "401 " + nick + " :No such nick");

    if (already_in(c, target->getFd()))
        return server.sendToClient(clientFd, "443 " + nick + " " + chan + " :is already on channel");

    invite(server, server.getClient(clientFd), *target, c);
}
