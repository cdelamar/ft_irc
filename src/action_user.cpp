#include "CommandHandler.hpp"
#include "utils.hpp"

void handleUser(Server &server, int clientFd, const Command &cmd)
{
    Client &client = server.getClient(clientFd);

    if (cmd.params.size() < 4)
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 461 * USER :Not enough parameters");
        return;
    }

    if (client.isRegistered())
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 462 * :Unauthorized command (already registered)");
        return;
    }

    client.setUsername(cmd.params[0]);
    client.setRealname(cmd.params[3]);

    client.checkRegistered();

    if (client.isRegistered())
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 001 " + client.getNickname() + " :Welcome to the Internet Relay Network, " + client.getNickname());
    }
}
