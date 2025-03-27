#include "Server.hpp"
#include "ServerException.hpp"
#include <cstring>
#include <arpa/inet.h>
#include <poll.h>       // pour struct pollfd et poll()
#include <vector>       // pour std::vector
#include <cstring>      // pour memset()
#include <unistd.h>     // pour close()
#include <fcntl.h>      // pour fcntl()
#include <iostream>     // pour les logs/debug
#include <sys/socket.h> // pour socket(), bind(), accept(), recv()
#include <netinet/in.h> // pour sockaddr_in, htons()

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
    // O_NONBLOCK - permet de rendre accept et recv () non bloquant (cf ESSENTIEL plus bas)


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


// pour plus de clarte sur cette fonction, voir 'start_example.cpp'
void Server::start()
{
	std::cout << "Server is listening on port " << _port << std::endl;

    sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientSocket;
    ssize_t bytesRead;

    std::cout << "Waiting for a client..." << std::endl;

    //accept() : le vigile a la porte d'entree

    // int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    // sockfd   : Ton socket serveur (celui qui a été bind() et listen()é)
    // addr     : Pointeur vers une structure qui recevra l’adresse du client
    // addrlen	: Taille de la structure (en entrée) et mise à jour (en sortie)
    clientSocket = accept(_servSocket, (sockaddr*)&clientAddr, &clientLen);
    if (clientSocket < 0)
        throw ServerException("Error : accept() has failed");

    std::cout << "Client connected !" << std::endl;

    char buffer[1024]; // par convention, car une ligne IRC (specialisation RFC) est de 512caracteres maxi
    std::memset(buffer, 0, sizeof(buffer)); // on nettoie le Ko au cas ou

    // recv() : comme read() pour les fichiers, recv() lis les data d'un socket TCP

    // ssize_t recv(int socket, void *buffer, size_t length, int flags);
    // socket	Le fd du socket client sur lequel tu attends des données
    // buffer	La zone mémoire où seront stockées les données reçues
    // length	La taille maximale que tu acceptes de recevoir
    // flags	Options (0 ici = comportement par défaut)
    bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    /// ESSENTIEL ///
    //             //
    // accept et recv sont BLOQUANT
    // c'est a dire que le serveur
    // restera fige tant qu'un client
    // ne se co pas (accept) ou n'envoie pas son msg (recv)


    if (bytesRead < 0)
        throw ServerException("Erreur: recv() a échoué");

    std::cout << "Message reçu du client : " << buffer << std::endl;

    close(clientSocket);
    std::cout << "Connexion client fermée." << std::endl;
}

void Server::pollLoop() {

    /* cette struct est inherente a <poll.h>, je n'invente rien ici
        je repompe juste le MAN

        struct pollfd {
            int fd;         // Le file descriptor à surveiller (ex : ton socket serveur ou un socket client)
            short events;   // Ce qu’on veut surveiller (lecture, écriture, etc.)
            short revents;  // Ce que poll() a détecté (rempli par poll())
        };
    */

	std::vector<struct pollfd> fds;

	struct pollfd servFd;
	servFd.fd = _servSocket;
	servFd.events = POLLIN;
	fds.push_back(servFd);

	while (true)
    {
		int ready = poll(&fds[0], fds.size(), -1);
		if (ready < 0) throw ServerException("poll() failed");

		for (size_t i = 0; i < fds.size(); ++i)
        {
			if (fds[i].revents & POLLIN)
            {
				if (fds[i].fd == _servSocket)
                {
				    sockaddr_in clientAddr;
                    socklen_t clientLen = sizeof(clientAddr);
                    int clientSocket = accept(_servSocket, (sockaddr*)&clientAddr, &clientLen);

                    if (clientSocket < 0)
                    {
                        std::cerr << "accept() failed (non bloquant, peut arriver) fd: " << _servSocket << std::endl;
                        continue;
                    }

                    std::cout << "Client connecté ! (fd: " << clientSocket << ")" << std::endl;

                    struct pollfd clientFd;
                    clientFd.fd = clientSocket;
                    clientFd.events = POLLIN;
                    fds.push_back(clientFd);
                    _clients.insert(std::make_pair(clientSocket, Client(clientSocket))); // le client garde pour lui le buffer lie au fd
				}
                else
                {
                    // partie a bien revoir en detail
                    char buffer[1024];
                    memset(buffer, 0, sizeof(buffer));
                    size_t msgRead = recv(fds[i].fd, buffer, sizeof(buffer - 1), 0);
					// Client existant parle

                    if (msgRead <= 0) {
                        std::cout << "Client " << fds[i].fd << " disconnected" << std::endl;
                        //fermeture du socket
                        close(fds[i].fd);

                        //suppression de l'entree du vecteur pollfd
                        fds.erase(fds.begin() + i);

                        //suppression du client dans la map
                        _clients.erase(fds[i].fd);

                        //on perd une place dans le vecteur donc on decremente l'indice, sinon BUG
                        --i;
                    }
                    else
                    {
                        //std::cout << "Message from client " << fds[i].fd << ": " << buffer << std::endl;
                        // ajoute les data recu direct dans le buffer
                        _clients[fds[i].fd].appendToBuffer(std::string(buffer, msgRead));

                        //tant que j'ai une commande complete >> extraction
                        while (_clients[fds[i].fd].hasCompleteCommand())
                        {
                                std::string cmd = _clients[fds[i].fd].extractCommand();
                                std::cout << "Commande complète du client " << fds[i].fd << " : " << cmd << std::endl;

                                // parsing prochainement
                        }
                    }
                }
			}
		}
	}

}