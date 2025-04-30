#include "CommandHandler.hpp"
#include "Server.hpp"
#include "ServerException.hpp"
#include "Command.hpp"
#include "Client.hpp"
#include <iostream>
#include "utils.hpp"

// TODO

/*
🔐 Authentification :

    PASS : mot de passe à l’entrée
    NICK : pseudonyme unique
    USER : informations utilisateur

👥 Interaction :

    JOIN : rejoindre un channel
    PART : quitter un channel
    PRIVMSG : envoyer un message (privé ou channel)
    PING / PONG : ping/pong de keep-alive

🔒 Commandes opérateurs :

    KICK : éjecter un membre du channel
    INVITE : inviter un utilisateur
    TOPIC : définir le sujet d’un channel
    MODE : changer les permissions du channel ou d’un utilisateur

🧱 Infrastructure technique :

    Sockets non bloquants
    Utilisation de poll() pour gérer plusieurs clients simultanés
    Norme C++98
    Respect du format IRC texte (terminaison \r\n, préfixes, trailing, etc.)
*/

/*
🔐 Authentification :

    PASS : mot de passe à l’entrée
    NICK : pseudonyme unique
    USER : informations utilisateur
*/

void Server::handleCommand(int clientFd, const Command &cmd)
{
	(void)clientFd;

    if (cmd.name.empty())
        return;

    // Bloquer toute commande tant que le pass n'est pas valide
    if (!getClient(clientFd).isPassSaved() && cmd.name != "PASS")
    {
        sendToClient(clientFd, ERR_PASSWDMISMATCH + std::string(" :Password required before executing commands"));
        return;
    }

    if (cmd.name == "PASS")
        handlePass(*this, clientFd, cmd);
    else if (cmd.name == "NICK")
        handleNick(*this, clientFd, cmd);
    else if (cmd.name == "USER")
        handleUser(*this, clientFd, cmd);
    else if (cmd.name == "JOIN")
        handleJoin(*this, clientFd, cmd);
    else if (cmd.name == "PRIVMSG")
        handlePrivMsg(*this, clientFd, cmd);
    else if (cmd.name == "TOPIC")
        handleTopic(*this, clientFd, cmd);
    else if (cmd.name == "PING")
        handlePing(*this, clientFd, cmd);
    else if (cmd.name == "PONG")
        handlePong(*this, clientFd, cmd);
    else if (cmd.name == "PART")
        handlePart(*this, clientFd, cmd);
    else if (cmd.name == "QUIT")
        handleQuit(*this, clientFd, cmd);
    else if (cmd.name == "KICK")
        handleKick(*this, clientFd, cmd);
    else if (cmd.name == "MODE")
        handleMode(*this, clientFd, cmd);
    else if (cmd.name == "INVITE")
        handleInvite(*this, clientFd, cmd);
    else
    {
        std::cout << "[INFO] Commande non implémentée : " << cmd.name << std::endl;
    }
}
