#include "CommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "utils.hpp"

void handleQuit(Server &server, int clientFd, const Command &cmd)
{
    Client &client = server.getClient(clientFd);

    std::string quitMessage = "Client Quit";
    if (!cmd.params.empty())
        quitMessage = cmd.params[0];

    std::string prefix = client.getNickname() + "!" + client.getUsername() + "@" + server.getHostname();
    std::string quitMsg = ":" + prefix + " QUIT :" + quitMessage;

    // Informer tous les membres des salons que ce client quitte
    const std::map<std::string, Channel> &channels = server.getChannels();
    for (std::map<std::string, Channel>::const_iterator it = channels.begin(); it != channels.end(); ++it)
    {
        const Channel &chan = it->second;
        if (chan.isMember(clientFd))
            chan.broadcast(server, quitMsg, clientFd);
    }

    // Ensuite, suppression logique
    server.leaveAllChannels(clientFd);
    server.removeClient(server.getPollFds(), clientFd);

    std::cout << "[INFO] Client déconnecté proprement (fd: " << clientFd << ")" << std::endl;
}
