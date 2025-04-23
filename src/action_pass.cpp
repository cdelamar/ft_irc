#include "CommandHandler.hpp"
#include "utils.hpp"


void handlePass(Server &server, int clientFd, const Command &cmd)
{
    Client &client = server.getClient(clientFd);

    if (client.isRegistered())
    {
        server.sendToClient(clientFd, ERR_ALREADYREGISTRED + std::string(" :Unauthorized command (already registered)"));
        return;
    }

    if (cmd.params.empty())
    {
        server.sendToClient(clientFd, ERR_NEEDMOREPARAMS + std::string(" PASS :Not enough parameters"));
        return;
    }

    const std::string &password = cmd.params[0];

    if (password != server.getPassword())
    {
        server.sendToClient(clientFd, ERR_PASSWDMISMATCH + std::string(" :Password incorrect"));
        close(clientFd); // peut etre un removeClient si ya des leaks
        return;
    }

    if (client.isPassSaved())
    {
        server.sendToClient(clientFd, ERR_ALREADYREGISTRED + std::string(" :Unauthorized command (already registered)"));
        return;
    }

    client.setPassSaved(true);
    std::cout << "[INFO] PASS accepted for fd: " << clientFd << std::endl;
    server.sendToClient(clientFd, "NOTICE AUTH :Password accepted");
}
