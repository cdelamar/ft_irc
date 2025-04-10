#include "Server.hpp"
#include "ServerException.hpp"

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
    addr.sin_port = htons(_port);

    if (bind(_servSocket, (sockaddr *)&addr, sizeof(addr)) < 0)
        throw ServerException("Error: bind() has failed");

    if (listen(_servSocket, 10) < 0)
        throw ServerException("Error: listen() has failed");
}

// AJOUTER DES CLIENTS AU SERVUER //

void Server::acceptNewClient(std::vector<struct pollfd> &fds)
{
	sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int clientSocket = accept(_servSocket, (sockaddr*)&clientAddr, &clientLen);

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

void Server::removeClient(std::vector<struct pollfd> &fds, size_t i)
{
	close(fds[i].fd);
	_clients.erase(fds[i].fd);
	fds.erase(fds.begin() + i);
	--i;
}

// GESTION DES MESSAGES ENVOYES PAR LES CLIENTS //

void Server::handleClientMessage(std::vector<struct pollfd> &fds, size_t i)
{
	char buffer[1024];
	std::memset(buffer, 0, sizeof(buffer));
	ssize_t msgRead = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);

	if (msgRead <= 0)
	{
		std::cout << "Client " << fds[i].fd << " disconnected" << std::endl;
		removeClient(fds, i);
        --i; // pour retirer un index dans le vector
		return;
	}
    //std::cout << "Message from client " << fds[i].fd << ": " << buffer << std::endl;
     // ajoute les data recu direct dans le buffer
	_clients[fds[i].fd].appendToBuffer(std::string(buffer, msgRead));

	while (_clients[fds[i].fd].hasCompleteCommand())
	{
		std::string cmdStr = _clients[fds[i].fd].extractCommand();
		Command parsed = parseCommand(cmdStr);

		std::cout << "[DEBUG] Parsed command: " << parsed.name << " (" << parsed.raw << ")" << std::endl;

		handleCommand(fds[i].fd, parsed); // appel du dispatcher
	}
}

// BOUCLE DU SERVEUR //

void Server::pollLoop()
{
	std::vector<struct pollfd> fds;
	struct pollfd servFd;

    /* cette struct est inherente a <poll.h>, je n'invente rien ici
        je repompe juste le MAN

        struct pollfd {
            int fd;         // Le file descriptor à surveiller (ex : ton socket serveur ou un socket client)
            short events;   // Ce qu’on veut surveiller (lecture, écriture, etc.)
            short revents;  // Ce que poll() a détecté (rempli par poll())
        };
    */

	servFd.fd = _servSocket;
	servFd.events = POLLIN;
	fds.push_back(servFd);

	while (true)
	{
		int ready = poll(&fds[0], fds.size(), -1);
		if (ready < 0)
			throw ServerException("poll() failed");

		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == _servSocket)
					acceptNewClient(fds);
				else
					handleClientMessage(fds, i);
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

// Dispatcher : partie importante qui changera souvent
void Server::handleCommand(int clientFd, const Command &cmd)
{
	(void)clientFd;
    if (cmd.name == "NICK")
    {
        std::cout << "[DEBUG] Handling NICK with param: " << (cmd.params.empty() ? "none" : cmd.params[0]) << std::endl;
        // TODO : stocker dans le client
    }
    else
    {
        std::cout << "[INFO] Commande non implémentée : " << cmd.name << std::endl;
    }
}