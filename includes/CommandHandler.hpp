#ifndef COMMAND_HANDLERS_HPP
#define COMMAND_HANDLERS_HPP

#include "Command.hpp"
#include "Server.hpp"

// Handler pour la commande NICK
void handleNick(Server &server, int clientFd, const Command &cmd);

// void handleUser(...);
// void handleJoin(...);

#endif
