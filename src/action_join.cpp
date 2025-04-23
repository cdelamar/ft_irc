#include "CommandHandler.hpp"
#include "utils.hpp"

void handleJoin(Server &server, int clientFd, const Command &cmd)
{
    if (cmd.params.empty())
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 461 * JOIN :Not enough parameters");
        return;
    }

    const std::string &chanName = cmd.params[0];
    Client &client = server.getClient(clientFd);

    if (!server.channelExists(chanName))
        server.createChannel(chanName);

    Channel &chan = server.getChannel(chanName);

    if (!chan.isMember(clientFd))
    {
        chan.addMember(client);

        if (chan.memberCount() == 1)
            chan.promoteToOperator(clientFd);

        // JOIN notification au format IRC
        std::string prefix = client.getNickname() + "!" + client.getUsername() + "@" + server.getHostname();
        server.sendToClient(client.getFd(), ":" + prefix + " JOIN :" + chan.getName());

        // Répondre au TOPIC (331 ou 332)
        const std::string &topic = chan.getTopic();
        if (topic.empty())
        {
            server.sendToClient(clientFd, ":" + server.getHostname() + " 331 " + client.getNickname() + " " + chan.getName() + " :No topic is set");
        }
        else
        {
            server.sendToClient(clientFd, ":" + server.getHostname() + " 332 " + client.getNickname() + " " + chan.getName() + " :" + topic);
        }

        // Répondre à la liste des NAMES
        std::vector<int> memberFds = chan.getMemberFds();
        std::string names;
        for (size_t i = 0; i < memberFds.size(); ++i)
        {
            names += server.getClient(memberFds[i]).getNickname();
            if (i + 1 < memberFds.size())
                names += " ";
        }

        server.sendToClient(clientFd, ":" + server.getHostname() + " 353 " + client.getNickname() + " = " + chan.getName() + " :" + names);
        server.sendToClient(clientFd, ":" + server.getHostname() + " 366 " + client.getNickname() + " " + chan.getName() + " :End of /NAMES list");
    }
}
