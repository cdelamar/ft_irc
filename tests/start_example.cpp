#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>


// Les nouveaux concepts

// Les sockets >>>		socket(), sockaddr_in, bind(), listen(), accept()
// Le multiplexing >>>	pollfd, poll(), revents, POLLIN
// nouveaux types >>>	socklen_t, INADDR_ANY, (sockaddr*)

int main () {

    // fais office de 'prise reseau' pour se connecter a internet
	int serv_fd = socket(AF_INET, SOCK_STREAM, 0); // ouvre un socket (TCP) sur le port standard  IRC qui est 6667
	// comment marche socket ici
    // AF_NET : reseau ipv4
    // SOCK_STREAM : protocole TCP
    // 0 : par defaut, suffisant pour notre exerice car l'OS choisi de lui meme

    if (serv_fd < 0)
	{
		std::cerr << "Socket error" << std::endl;
		return 1;
	}

	//reseau adresse ipv4

    // sockaddr_in fait office de struct pre concu expres pour la
    // manipulation d'addresse IP

    // struct sockaddr_in {
    // short sin_family;           // Type de l'adresse (AF_INET pour IPv4)
    // unsigned short sin_port;    // Numéro de port
    // struct in_addr sin_addr;    // Adresse IP
    // char sin_zero[8];           // Remplissage pour compatibilité
    // };
	sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr)); // pas indispensable mais plus safe

	/// A VOIR A PARTIR DICI

    serv_addr.sin_family = AF_INET;             // on defini notre adresse comme IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY;     // on accepte tout type d'IP (localhost, wifi ou ethernet etc etc)
    serv_addr.sin_port = htons(6667);           // port 6667, en format reseau

    // POURQUOI 6667
    // Par convention (RFC 2812), les serveurs IRC écoutent
    // sur les ports 6660 à 6669, et 6667 devient le plus couramment utilisé.

    // on cherche ensuite a 'lier'
    // cette adresse au socket precemment declarer
    // bind(FD du socket, addresse IP, taille du socket)
    if (bind(serv_fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Erreur bind" << std::endl;
        return 1;
    }

    // on met notre socket en ecoute, pret a recevoir des messages
    // 10 etant la taille limite de la file d'attente des client en attente de co
    if (listen(serv_fd, 10) < 0)
    {
        std::cerr << "Erreur listen" << std::endl;
        return 1;
    }

    std::cout << "Serveur en attente sur le port 6667" << std::endl;

    // a partir d'ici
    // 1. on a cree un socket
    // 2. on a cree une adresse
    // 3. on les a lier (bind)
    // 4. on met le socket en ecoute (listen)

pollfd fds[1];
fds[0].fd = serv_fd;
fds[0].events = POLLIN;

// pour Server :: start

int ret;
sockaddr_in client_addr;
socklen_t client_len;
int client_fd;

while (true)
{
    std::cout << "Attente d'un événement..." << std::endl;
    ret = poll(fds, 1, -1);
    if (ret < 0)
    {
        std::cerr << "Erreur poll" << std::endl;
        break;
    }

    if (fds[0].revents & POLLIN)
    {
        client_len = sizeof(client_addr);
        client_fd = accept(serv_fd, (sockaddr*)&client_addr, &client_len);
        if (client_fd >= 0)
        {
            std::cout << "Nouveau client connecté !" << std::endl;
            //close(client_fd);
        }
    }
}

    /*
    [Serveur TCP sur 6667]
        |
   poll() attend
        |
   Nouveau client ?
        |
     accept()
        |
   Message "Client connecté"
        |
   close(client_fd)
        |
   Retour à poll()
    */

    close(serv_fd);
    return 0;
}