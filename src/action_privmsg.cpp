#include "CommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "utils.hpp"

static bool isPrivMsg(const std::string &target)
{
    return !target.empty() && target[0] != '#';
}

static bool validPrivMsg(Server &server, int fd, const Command &cmd)
{
    Client &sender = server.getClient(fd);

    if (cmd.params.size() < 2)
    {
        server.sendToClient(fd, ":" + server.getHostname() + " 461 " + sender.getNickname() + " PRIVMSG :Not enough parameters");
        return false;
    }

    if (cmd.params[1].empty())
    {
        server.sendToClient(fd, ":" + server.getHostname() + " 412 " + sender.getNickname() + " :No text to send");
        return false;
    }

    return true;
}

static std::string msgPrefix(const Client &client, const std::string &hostname)
{
    return client.getNickname() + "!" + client.getUsername() + "@" + hostname;
}

static std::string fullMsg(const std::string &prefix, const std::string &target, const std::string &message)
{
    return ":" + prefix + " PRIVMSG " + target + " :" + message;
}

static void channelBroadcast(Server &server, int senderFd, const std::string &channelName, const std::string &message)
{
    Client &sender = server.getClient(senderFd);
    if (!server.channelExists(channelName))
    {
        server.sendToClient(senderFd, ":" + server.getHostname() + " 403 " + channelName + " :No such channel");
        return;
    }

    Channel &chan = server.getChannel(channelName);

    if (!chan.isMember(senderFd))
    {
        server.sendToClient(senderFd, ":" + server.getHostname() + " 404 " + channelName + " :Cannot send to channel");
        return;
    }

    std::string prefix = msgPrefix(sender, server.getHostname());
    std::string response = fullMsg(prefix, channelName, message);
    chan.broadcast(server, response, senderFd);
}

static void dispatchPrivMsgToUser(Server &server, int senderFd, const std::string &targetNick, const std::string &message)
{
    const std::map<int, Client> &clients = server.getClients();
    Client &sender = server.getClient(senderFd);
    bool found = false;

    std::string prefix = msgPrefix(sender, server.getHostname());
    std::string response = fullMsg(prefix, targetNick, message);

    for (std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.getNickname() == targetNick)
        {
            server.sendToClient(it->first, response);
            found = true;
            break;
        }
    }

    if (!found)
    {
        server.sendToClient(senderFd, ":" + server.getHostname() + " 401 " + targetNick + " :No such nick/channel");
    }
}

void handlePrivMsg(Server &server, int clientFd, const Command &cmd)
{
    if (!validPrivMsg(server, clientFd, cmd))
        return;

    const std::string &target = cmd.params[0];
    const std::string &message = cmd.params[1];

    if (isPrivMsg(target))
        dispatchPrivMsgToUser(server, clientFd, target, message);
    else
        channelBroadcast(server, clientFd, target, message);
}