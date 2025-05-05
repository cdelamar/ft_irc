#include "CommandHandler.hpp"
#include "utils.hpp"

void handlePart(Server &server, int clientFd, const Command &cmd)
{
    Client &client = server.getClient(clientFd);

    if (cmd.params.empty())
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 461 * PART :Not enough parameters");
        return;
    }

    const std::string &chanName = cmd.params[0];

    std::string reason;
    if (cmd.params.size() > 1)
        reason = cmd.params[1];

    if (!server.channelExists(chanName))
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 403 " + chanName + " :No such channel");
        return;
    }

    Channel &chan = server.getChannel(chanName);

    if (!chan.isMember(clientFd))
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 442 " + chanName + " :You're not on that channel");
        return;
    }

		// au dessus le parsing habituel des commandes

    // Message de PART au format IRC
    std::string prefix = client.getNickname() + "!" + client.getUsername() + "@" + server.getHostname();
    std::string partMsg = ":" + prefix + " PART " + chanName;
    if (!reason.empty())
        partMsg += " :" + reason;

    chan.broadcast(server, partMsg, clientFd); // prevenir tout le monde sauf celui qui part
    server.sendToClient(clientFd, partMsg);     // prevenir celui qui part

    chan.removeMember(clientFd);

    // Si le channel est vide apres le départ -> on peut le supprimer du serveur
    if (chan.memberCount() == 0)
    {
        server.getChannels().erase(chanName);
        std::cout << "[INFO] Salon supprimé : " << chanName << std::endl;
    }
}
