#include "CommandHandler.hpp"
#include "utils.hpp"

void handlePing(Server &server, int clientFd, const Command &cmd)
{
    Client &client = server.getClient(clientFd);

    if (cmd.params.empty())
    {
        server.sendToClient(clientFd, ":" + server.getHostname() + " 409 " + client.getNickname() + " PING :No origin specified");
        return;
    }

    const std::string &token = cmd.params[0];

    server.sendToClient(clientFd, "PONG :" + token);
}

void handlePong(Server &server, int clientFd, const Command &cmd)
{
    (void)server; // Rien a faire ici pour l'instant
    (void)clientFd;
    (void)cmd;

    // Dans une vrai architecture, on pourrait ici
    //- Remettre à zero un timer pour eviter une deco du client
}