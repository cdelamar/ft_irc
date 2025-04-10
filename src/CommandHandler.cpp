#include "CommandHandler.hpp"
#include "Server.hpp"
#include <iostream>

void handleNick(Server &server, int clientFd, const Command &cmd)
{
    if (cmd.params.empty()) {
        std::cerr << "[ERROR] NICK without args" << std::endl;
        return;
    }

    const std::string &newNick = cmd.params[0];

    if (server.isNicknameTaken(newNick)) {
        std::cerr << "[ERROR] Nickname already taken : " << newNick << std::endl;
        return;
    }

    server.getClient(clientFd).setNickname(newNick);
    std::cout << "[INFO] Client fd " << clientFd << " → Nickname set to '" << newNick << "'" << std::endl;
}
