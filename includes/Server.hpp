#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>
#include "Client.hpp"
#include <vector>   // pour std::vector
#include <cstddef>  // pour size_t
#include <poll.h>   // pour struct pollfd

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

public:
	Server(int port, const std::string &password);
	Server(const Server &src);
    Server &operator=(const Server &src);
    ~Server();

	// accept et recv
	void pollLoop();
};

#endif