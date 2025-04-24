#include "CommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "utils.hpp"

static std::string stringJoin (const std::vector<std::string> &params, size_t index)
{
    std::string res;
    size_t i = index;

    while (i < params.size())
    {
        res += params[i];
        if (i + 1 < params.size())
            res += " ";
        i++;
    }

    return res;
}

void handleTopic(Server &server, int clientFd, const Command &cmd)
{
    Client &client = server.getClient(clientFd);

    if (cmd.params.empty())
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 461 " + client.getNickname() + " TOPIC :Not enough parameters");
        return;
    }

    const std::string &channelName = cmd.params[0];

    if (!server.channelExists(channelName))
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 403 " + channelName + " :No such channel");
        return;
    }

    Channel &channel = server.getChannel(channelName);

    if (!channel.isMember(clientFd))
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 442 " + channelName + " :You're not on that channel");
        return;
    }

    if (cmd.params.size() == 1)
    {
        const std::string &topic = channel.getTopic();
        if (topic.empty())
            server.sendToClient(clientFd, ":" + server.getHostname() + " 331 " + client.getNickname() + " " + channelName + " :No topic is set");
        else
            server.sendToClient(clientFd, ":" + server.getHostname() + " 332 " + client.getNickname() + " " + channelName + " :" + topic);
        return;
    }

    if (!channel.isOperator(clientFd))
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 482 " + channelName + " :You're not channel operator");
        return;
    }

    const std::string &newTopic = stringJoin(cmd.params, 1);
    channel.setTopic(newTopic);

    std::string prefix = client.getNickname() + "!" + client.getUsername() + "@" + server.getHostname();
    std::string topicMsg = ":" + prefix + " TOPIC " + channelName + " :" + newTopic;

    channel.broadcast(server, topicMsg, -1);
}