#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>
#include "Client.hpp"
#include <vector>   // pour std::vector
#include <cstddef>  // pour size_t
#include <poll.h>   // pour struct pollfd
#include <cstring>
#include <arpa/inet.h>
#include <poll.h>       // pour struct pollfd et poll()
#include <vector>       // pour std::vector
#include <cstring>      // pour memset()
#include <unistd.h>     // pour close()
#include <fcntl.h>      // pour fcntl()
#include <iostream>     // pour les logs/debug
#include <sstream>
#include <sys/socket.h> // pour socket(), bind(), accept(), recv()
#include <netinet/in.h> // pour sockaddr_in, htons()
#include <algorithm>	// pour std::transform
#include "Command.hpp"


class Server
{
private:
	int _servSocket;
	int _port;
	std::string _password;
	std::map<int, Client> _clients;

	void initSocket();

	//inherente a 'pollLoop()'
	void acceptNewClient(std::vector<struct pollfd> &fds);
	void handleClientMessage(std::vector<struct pollfd> &fds, size_t i);
	void removeClient(std::vector<struct pollfd> &fds, size_t i);

	//inherente a 'handleCommand'
	std::vector<std::string> tokenizeCommand(const std::string &command);

public:
	Server(int port, const std::string &password);
	Server(const Server &src);
    Server &operator=(const Server &src);
    ~Server();

	// accept et recv
	void pollLoop();
	//parsing des commandes
	Command parseCommand(const std::string &rawCommand);
	void handleCommand(int clientFd, const Command &cmd);
	//void handleCommand(int clientFd, const std::string &rawcommand);
};

#endif