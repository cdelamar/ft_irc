#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "Command.hpp"

static Client* findByNick(Server &srv, const std::string &nick)
{
    std::vector<int> fds = srv.getClientFds();
    for (size_t i = 0; i < fds.size(); ++i)
    {
        Client &c = srv.getClient(fds[i]);
        if (c.getNickname() == nick)
            return &c;
    }
    return 0;
}

void handleInvite(Server &srv, int fd, const Command &cmd)
{
    if (cmd.params.size() < 2)
        return srv.sendToClient(fd, "461 INVITE :Need more parameters");

    std::string nick = cmd.params[0];
    std::string chan = cmd.params[1];

    if (!srv.channelExists(chan))
        return srv.sendToClient(fd, "403 " + chan + " :No such channel");

    Channel &c = srv.getChannel(chan);

    if (!c.isMember(fd))
        return srv.sendToClient(fd, "442 " + chan + " :You're not on that channel");

    if (!c.isOperator(fd))
        return srv.sendToClient(fd, "482 " + chan + " :You're not channel operator");

    Client *target = findByNick(srv, nick);
    if (!target)
        return srv.sendToClient(fd, "401 " + nick + " :No such nick");

    if (c.isMember(target->getFd()))
        return srv.sendToClient(fd, "443 " + nick + " " + chan + " :is already on channel");

    // Ajouter à la liste des invités (vector)
    c.addInvite(target->getFd());

    // Confirmation à l'invitant
    srv.sendToClient(fd, "341 " + nick + " " + chan);

    // Notification à l'invité
    Client &sender = srv.getClient(fd);
    std::string msg = ":" + sender.getNickname() + "!" + sender.getUsername() + "@localhost INVITE " + nick + " :" + chan;
    srv.sendToClient(target->getFd(), msg);
}
