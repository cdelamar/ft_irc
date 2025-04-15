#include "CommandHandler.hpp"
#include "Server.hpp"
#include "ServerException.hpp"
#include "Command.hpp"
#include "Client.hpp"
#include <iostream>

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

void handleNick(Server &server, int clientFd, const Command &cmd)
{
    Client &client = server.getClient(clientFd);

    if (cmd.params.empty()) {
        server.sendToClient(clientFd, ERR_NONICKNAMEGIVEN + std::string(" NICK :No nickname given"));
        return;
    }

    const std::string &newNick = cmd.params[0];

    // (TODO : un validateur de nickname pour retourner 432 si besoin

    if (server.isNicknameTaken(newNick)) {
        server.sendToClient(clientFd, ERR_NICKNAMEINUSE + std::string(" " + newNick + " :Nickname is already in use"));
        return;
    }

    client.setNickname(newNick);
    std::cout << "[INFO] Client fd " << clientFd << " → Nickname set to '" << newNick << "'" << std::endl;
}

void handleUser(Server &server, int clientFd, const Command &cmd)
{
/*
Recap des parametres pour la commande IRC USER :

Format general :
    USER <username> <unused> <unused> :<realname>

Exemple typique :
    USER marston 0 * :John Marston

Apres parsing, on obtient :
    cmd.params[0] = "marston";       // username (utilise pour l'identite technique)
    cmd.params[1] = "0";             // obsolete, requis par le protocole, on ignore
    cmd.params[2] = "*";             // obsolete, ancien nom de serveur, ignore aussi
    cmd.params[3] = "John Marston";  // realname (nom affiche aux autres utilisateurs)

Note :
- Les champs 1 et 2 sont requis par la RFC 2812 mais ne sont pas a exploiter pour le projet
- Le champ realname (params[3]) peut contenir des espaces grace au prefixe ':'
*/
    Client &client = server.getClient(clientFd);

    if (cmd.params.size() < 4)
    {
        server.sendToClient(clientFd, ERR_NEEDMOREPARAMS + std::string(" USER :Not enough parameters"));
        return;
    }

    if (client.isRegistered())
    {
        server.sendToClient(clientFd, ERR_ALREADYREGISTRED + std::string(" :Unauthorized command (already registered)"));
        return;
    }

    client.setUsername(cmd.params[0]);
    client.setRealname(cmd.params[3]);

    client.checkRegistered();

    std::cout << "[INFO] USER received. Username: " << cmd.params[0]
              << ", Realname: " << cmd.params[3] << std::endl;

    if (client.isRegistered())
        std::cout << "[INFO] Client " << clientFd << " is now registered (NICK + USER done)" << std::endl;
}

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



// Dispatcher : partie importante qui changera souvent
void Server::handleCommand(int clientFd, const Command &cmd)
{
	(void)clientFd;

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
    else
    {
        std::cout << "[INFO] Commande non implémentée : " << cmd.name << std::endl;
    }
}