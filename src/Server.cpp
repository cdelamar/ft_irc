#include "Server.hpp"
#include "ServerException.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

Server::Server(int port, const std::string &password)
    : _servSocket(-1), _port(port), _password(password)
{
    initSocket();
}

Server::Server(const Server &other)
    : _servSocket(other._servSocket), _port(other._port), _password(other._password)
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

    if (bytesRead < 0)
        throw ServerException("Erreur: recv() a échoué");

    std::cout << "Message reçu du client : " << buffer << std::endl;

    close(clientSocket);
    std::cout << "Connexion client fermée." << std::endl;
}