#include "Server.hpp"
#include "ServerException.hpp"
#include "CommandHandler.hpp"
#include "Channel.hpp"

Server::Server(int port, const std::string &password)
    : _servSocket(-1), _port(port), _password(password) ,_clients()
{
    initSocket();
}

Server::Server(const Server &other)
    : _servSocket(other._servSocket), _port(other._port), _password(other._password), _clients(other._clients)
{
}

Server &Server::operator=(const Server &other)
{
    if (this != &other)
    {
        _servSocket = other._servSocket;
        _port = other._port;
        _password = other._password;
    }
    return *this;
}

Server::~Server()
{
    if (_servSocket >= 0)
        close(_servSocket);
}

void Server::initSocket()
{
	// fais office de 'prise reseau' pour se connecter a internet
	_servSocket = socket(AF_INET, SOCK_STREAM, 0); // ouvre un socket (TCP) sur le port standard  IRC qui est 6667

	// comment marche socket ici
    // AF_NET : reseau ipv4
    // SOCK_STREAM : protocole TCP
    // 0 : par defaut, suffisant pour notre exerice car l'OS choisi de lui meme
    if (_servSocket < 0)
        throw ServerException("Error: socket() has failed");

    // fcntl() : file control
    // fonction qui permet de gerer des FD
    // ici, fcntl() nous permet de config notre socket en 'non-bloquant'
    // F_SETFL pour "Set File status Flags" - pour modifier le comportement du fd
    // O_NONBLOCK - permet de rendre accept et recv () non bloquant (cf ESSENTIEL plus bas
    if (fcntl(_servSocket, F_SETFL, O_NONBLOCK) < 0)
       throw ServerException("Error: fcntl() failed to set non-blocking mode");
    int opt = 1;

    // sans ce if(), le code est a la merci de TIME_WAIT
    // TIME_WAIT, c'est un etat TCP imposer par le kernel
    if (setsockopt(_servSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
       throw ServerException("Error: setsockopt() has failed");

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(_port));

    if (bind(_servSocket, reinterpret_cast<sockaddr *> (&addr), sizeof(addr)) < 0)
        throw ServerException("Error: bind() has failed");

    if (listen(_servSocket, 10) < 0)
        throw ServerException("Error: listen() has failed");
}

// AJOUTER DES CLIENTS AU SERVUER //

void Server::acceptNewClient(std::vector<struct pollfd> &fds)
{
	sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int clientSocket = accept(_servSocket, reinterpret_cast<sockaddr *> (&clientAddr), &clientLen);

	if (clientSocket < 0)
	{
		std::cerr << "accept() failed (non bloquant, peut arriver) fd: " << _servSocket << std::endl;
		return;
	}

	std::cout << "Client connecté ! (fd: " << clientSocket << ")" << std::endl;

	struct pollfd clientFd;
	clientFd.fd = clientSocket;
	clientFd.events = POLLIN;
	fds.push_back(clientFd);

	_clients.insert(std::make_pair(clientSocket, Client(clientSocket)));
}

// SUPPRIMER DES CLIENT DU SERVEUR //

/*void Server::removeClient(std::vector<struct pollfd> &fds, size_t i)
{
	close(fds[i].fd);
	_clients.erase(fds[i].fd);
	fds.erase(fds.begin() + i);
	--i;
}*/

// Version JPL - MISRA-C++ - NASA

void Server::removeClient(std::vector<struct pollfd> &fds, int fd)
{
    if (fd >= 0)
    {
        if (close(fd) < 0)
            std::cerr << "[ERROR] close() failed for fd " << fd << std::endl;
    }

    _clients.erase(fd);

    std::vector<struct pollfd>::iterator itFd = fds.begin();
    while (itFd != fds.end())
    {
        if (itFd->fd == fd)
        {
            itFd = fds.erase(itFd);
            break;
        }
        else
            ++itFd;
    }
}


// GESTION DES MESSAGES ENVOYES PAR LES CLIENTS //

void Server::handleClientMessage(std::vector<struct pollfd> &fds, int i)
{
	char buffer[1024];
	std::memset(buffer, 0, sizeof(buffer));
	ssize_t msgRead = recv(fds[static_cast<size_t>(i)].fd, buffer, sizeof(buffer) - 1, 0);

	if (msgRead <= 0)
	{
		std::cout << "Client " << fds[static_cast<size_t>(i)].fd << " disconnected" << std::endl;
		removeClient(fds, fds[static_cast<size_t>(i)].fd);
       // --i; // pour retirer un index dans le vector
		return;
	}
    //std::cout << "Message from client " << fds[i].fd << ": " << buffer << std::endl;
     // ajoute les data recu direct dans le buffer
	_clients[fds[static_cast<size_t>(i)].fd].appendToBuffer(std::string(buffer, static_cast<size_t>(msgRead)));

    while (_clients[fds[static_cast<size_t>(i)].fd].hasCompleteCommand())
    {
        std::string cmdStr = _clients[fds[static_cast<size_t>(i)].fd].extractCommand();
        Command parsed = parseCommand(cmdStr);

        std::cout << "[DEBUG] Parsed command: " << parsed.name << " (" << parsed.raw << ")" << std::endl;

        handleCommand(fds[static_cast<size_t>(i)].fd, parsed);
    }
}

// BOUCLE DU SERVEUR //
    /* cette struct est inherente a <poll.h>, je n'invente rien ici
        je repompe juste le MAN

        struct pollfd {
            int fd;         // Le file descriptor à surveiller (ex : ton socket serveur ou un socket client)
            short events;   // Ce qu’on veut surveiller (lecture, écriture, etc.)
            short revents;  // Ce que poll() a détecté (rempli par poll())
        };
    */

    /*
    ┌──────────────────────────── pollLoop() : coeur du serveur ──────────────────────────────┐
    │                                                                                        │
    │  Structure de données utilisée :                                                       │
    │                                                                                        │
    │  +------------------------+      +------------------------------+                      │
    │  | vector<pollfd> fds     |      | map<int, Client> _clients     |                     │
    │  | (pour poll())          |      | (données logiques)           |                      │
    │  +------------------------+      +------------------------------+                      │
    │  |                        |      |                              |                      │
    │  | [0] fd = 3             |----->| Clé : 3                      |                      │
    │  |     events = POLLIN    |      | Valeur : Client              |                      │
    │  |                        |      |   fd        = 3              |                      │
    │  | [1] fd = 4             |----->| Clé : 4                      |                      │
    │  |     events = POLLIN    |      | Valeur : Client              |                      │
    │  +------------------------+      |   nickname  = "Johnn"        |                      │
    │                                 +------------------------------+                       │
    │                                                                                        │
    │  Le file descriptor (fd) est la clé commune entre le réseau (poll) et le client        │
    │                                                                                        │
    └────────────────────────────────────────────────────────────────────────────────────────┘
    */

    /*
    ╔═══════════════════════╤════════════════════════════════════════════════════════╗
    ║      Données IRC      │         Chemin des données vers ton programme         ║
    ╟───────────────────────┼────────────────────────────────────────────────────────╢
    ║ CLIENT IRC (ex: irssi)│ Tape une commande : "PRIVMSG #chan :Salut\r\n"        ║
    ╟───────────────────────┼────────────────────────────────────────────────────────╢
    ║ Réseau (TCP/IP)       │ Le message est découpé en paquets TCP,                ║
    ║                       │ limité par la **fenêtre TCP** (gérée par l'OS)        ║
    ╟───────────────────────┼────────────────────────────────────────────────────────╢
    ║ Buffer TCP (kernel)   │ Les paquets valides arrivent dans le buffer réseau    ║
    ║                       │ du **système** (pile TCP Linux)                       ║
    ╟───────────────────────┼────────────────────────────────────────────────────────╢
    ║ recv() (ton code)     │ Tu récupères les octets disponibles avec `recv()`     ║
    ╟───────────────────────┼────────────────────────────────────────────────────────╢
    ║ _msgBuffer (toi)      │ Tu stockes les octets reçus, et tu détectes           ║
    ║                       │ **si une commande IRC est complète** (via \r\n)       ║
    ╟───────────────────────┼────────────────────────────────────────────────────────╢
    ║ extractCommand()      │ Une fois \r\n détecté, tu extrais la commande         ║
    ║                       │ pour la parser et la traiter                          ║
    ╚═══════════════════════╧════════════════════════════════════════════════════════╝

    🧠 Résumé : Tu ne contrôles ni la taille, ni la vitesse, ni l’ordre de réception.
    Ton `_msgBuffer` est là pour garantir que tu traites les messages IRC **correctement**, même
    s’ils arrivent coupés, en vrac, ou fusionnés dans le même `recv()`.
    */


void Server::pollLoop()
{
	std::vector<struct pollfd> fds;
	struct pollfd servFd;
	servFd.fd = _servSocket;
	servFd.events = POLLIN;
	fds.push_back(servFd);

	while (true)
	{
		int ready = poll(&fds[0], static_cast<nfds_t>(fds.size()), -1);
		if (ready < 0)
			throw ServerException("poll() failed");

		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == _servSocket)
					acceptNewClient(fds);
				else
					handleClientMessage(fds, static_cast<int>(i));
			}
		}
	}
}


// PARSING

Command Server::parseCommand(const std::string &rawCommand)
{
	// TODO : documenter cette fonction

    Command cmd;
    cmd.raw = rawCommand;

    std::istringstream iss(rawCommand);
    std::string word;

    // Lire la commande
    iss >> cmd.name;
    std::transform(cmd.name.begin(), cmd.name.end(), cmd.name.begin(), ::toupper);

    // Lire les param : gnl mais en C++
    while (iss >> word)
    {
        if (word[0] == ':') {
            std::string trailing = word.substr(1);
            std::string rest;
            std::getline(iss, rest);
            cmd.params.push_back(trailing + rest);
            break;
        }
        cmd.params.push_back(word);
    }

    return cmd;
}

Client &Server::getClient(int fd)
{
    return _clients[fd];
}

bool Server::isNicknameTaken(const std::string &nickname, int excludeFd)
{
    std::map<int, Client>::const_iterator it = _clients.begin();

    while (it != _clients.end())
    {
        if (it->first != excludeFd && it->second.getNickname() == nickname)
            return true;

        ++it;
    }
    return false;
}

const std::string &Server::getPassword() const { return _password; }

/*
void Server::sendToClient(int fd, const std::string &msg)
{
    std::string finalMsg = msg + "\r\n";
    const char *data = finalMsg.c_str();
    size_t totalSent = 0;
    size_t toSend = finalMsg.size();

    while (totalSent < toSend)
    {
        ssize_t sent = send(fd, data + totalSent, toSend - totalSent, 0);
        if (sent < 0)
        {
            std::cerr << "[ERROR] send() failed for fd " << fd << std::endl;
            break;
        }
        totalSent += static_cast<size_t>(sent);
    }
}*/

void Server::sendToClient(int fd, const std::string &msg)
{
    std::string finalMsg = msg + "\r\n";
    const char *data = finalMsg.c_str();
    size_t totalSent = 0;
    size_t toSend = finalMsg.size();

    while (totalSent < toSend)
    {
        ssize_t sent = send(fd, data + totalSent, toSend - totalSent, 0);
        if (sent < 0)
        {
            std::cerr << "[ERROR] send() failed for fd " << fd << std::endl;
            // ici correction :
            std::vector<struct pollfd> &fds = getPollFds(); // attention, getPollFds() actuel retourne un dummy
            removeClient(fds, fd); // deco propre
            return;
        }
        totalSent += static_cast<size_t>(sent);
    }
}

const std::string &Server::getHostname() const
{
    static const std::string hostname = "ircserv"; // ou gethostname() réel si besoin
    return hostname;
}

bool Server::channelExists(const std::string &name) const
{
    return (_channels.find(name) != _channels.end());
}

Channel &Server::getChannel(const std::string &name)
{
    // .at() lève std::out_of_range si le salon n’existe pas : comportement clair
    return _channels.at(name);
}

const std::map<int, Client> &Server::getClients() const
{
    return _clients;
}

std::map<std::string, Channel> &Server::getChannels()
{
    return _channels;
}

void Server::createChannel(const std::string &name)
{
    // n'insère que s'il n'existe pas déjà
    if (!channelExists(name))
        _channels.insert(std::make_pair(name, Channel(name)));
}

void Server::leaveAllChannels(int clientFd)
{
    std::map<std::string, Channel>::iterator it;
    it = _channels.begin();

    while (it != _channels.end())
    {
        it->second.removeMember(clientFd);
        ++it;
    }

    return;
}

std::vector<struct pollfd> &Server::getPollFds() // TODO : TEMPORAIRE a fix plus tard c'est moche
{
    static std::vector<struct pollfd> dummy; // fallback si jamais, mais en vrai on va passer la vraie fds
    return dummy;
}
