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
#include "Channel.hpp"


class Server
{
private:
	int								_servSocket;
	int								_port;
	std::string						_password;
	std::map<int, Client>			_clients;
	std::map<std::string, Channel>	_channels;

	void initSocket();

	//inherente a 'pollLoop()'
	void acceptNewClient(std::vector<struct pollfd> &fds);
	void handleClientMessage(std::vector<struct pollfd> &fds, int i);
	void removeClient(std::vector<struct pollfd> &fds, int fd);

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

	Client &getClient(int fd);
	// bool isNicknameTaken(const std::string &nickname);
	bool isNicknameTaken(const std::string &nickname, int excludeFd);


	//channel
	bool channelExists(const std::string &name) const;
	Channel &getChannel(const std::string &name);
	void createChannel(const std::string &name);

	const std::string &getPassword() const;
	const std::string &getHostname() const;

	void sendToClient(int fd, const std::string &msg);
};

#endif
