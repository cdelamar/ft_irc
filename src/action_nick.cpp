#include "CommandHandler.hpp"
#include "utils.hpp"

void handleNick(Server &server, int clientFd, const Command &cmd)
{
    Client &client = server.getClient(clientFd);

    if (cmd.params.size() != 1)
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 431 * :No nickname given");
        return;
    }

    const std::string &newNick = cmd.params[0];

    if (client.getNickname() == newNick)
        return;

    if (!isValidNickname(newNick))
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 432 * " + newNick + " :Erroneous nickname");
        return;
    }

    if (server.isNicknameTaken(newNick, clientFd))
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 433 * " + newNick + " :Nickname is already in use");
        return;
    }

    client.setNickname(newNick);

    server.sendToClient(clientFd, ":" + server.getHostname() + " NICK :" + newNick);

    client.checkRegistered();

    if (client.isRegistered())
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 001 " + client.getNickname() + " :Welcome to the Internet Relay Network, " + client.getNickname());
    }
}