#include "CommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "utils.hpp"

static bool kickParsing(Server &server, Client &sender, const std::string &channelName)
{
    if (!server.channelExists(channelName))
    {
        server.sendToClient(sender.getFd(), ":" + server.getHostname() + " 403 " + channelName + " :No such channel");
        return false;
    }

    Channel &channel = server.getChannel(channelName);

    if (!channel.isMember(sender.getFd()))
    {
        server.sendToClient(sender.getFd(), ":" + server.getHostname() + " 442 " + channelName + " :You're not on that channel");
        return false;
    }

    if (!channel.isOperator(sender.getFd()))
    {
        server.sendToClient(sender.getFd(), ":" + server.getHostname() + " 482 " + channelName + " :You're not channel operator");
        return false;
    }

    return true;
}

static int FdFinder(Server &server, const std::string &targetNick)
{
    const std::map<int, Client> &clients = server.getClients();
    for (std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.getNickname() == targetNick)
            return it->first;
    }
    return -1;
}

static void notifyKick(Server &server, Channel &channel, Client &sender, const Client &target, const std::string &reason)
{
    std::string prefix = sender.getNickname() + "!" + sender.getUsername() + "@" + server.getHostname();
    std::string kickMsg = ":" + prefix + " KICK " + channel.getName() + " " + target.getNickname() + " :" + reason;

    channel.broadcast(server, kickMsg, -1);
    server.sendToClient(target.getFd(), kickMsg);
}

static void updateNamesList(Server &server, Channel &channel)
{
    std::vector<int> members = channel.getMemberFds();
    std::string names;

    for (size_t i = 0; i < members.size(); ++i)
    {
        names += server.getClient(members[i]).getNickname();
        if (i + 1 < members.size())
            names += " ";
    }

    for (size_t i = 0; i < members.size(); ++i)
    {
        int fd = members[i];
        server.sendToClient(fd, ":" + server.getHostname() + " 353 " + server.getClient(fd).getNickname() + " = " + channel.getName() + " :" + names);
        server.sendToClient(fd, ":" + server.getHostname() + " 366 " + server.getClient(fd).getNickname() + " " + channel.getName() + " :End of /NAMES list");
    }
}


void handleKick(Server &server, int clientFd, const Command &cmd)
{
    Client &sender = server.getClient(clientFd);

    if (cmd.params.size() < 2)
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 461 * KICK :Not enough parameters");
        return;
    }

    const std::string &channelName = cmd.params[0];
    const std::string &targetNick = cmd.params[1];

    if (!kickParsing(server, sender, channelName))
        return;

    int targetFd = FdFinder(server, targetNick);
    if (targetFd == -1)
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 401 " + targetNick + " :No such nick/channel");
        return;
    }

    Channel &channel = server.getChannel(channelName);
    if (!channel.isMember(targetFd))
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 441 " + targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }

    const Client &target = server.getClient(targetFd);

    std::string reason;
    if (cmd.params.size() > 2)
        reason = cmd.params[2];
    else
        reason = "Kicked";

    notifyKick(server, channel, sender, target, reason);

    channel.removeMember(targetFd);

    // Correction : envoyer la nouvelle liste de membres (NAMES)
    updateNamesList(server, channel);

    std::cout << "[INFO] " << sender.getNickname() << " kicked " << targetNick << " from " << channelName << std::endl;
}
