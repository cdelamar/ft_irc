#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>
#include "Client.hpp"

class Server
{
private:
	int _servSocket;
	int _port;
	std::string _password;
	std::map<int, Client> _clients;

	void initSocket();

public:
	Server(int port, const std::string &password);
	Server(const Server &src);
    Server &operator=(const Server &src);
    ~Server();

	//obsolete, mais utile pour les tests
    void start(); // test simple : afficher que le serveur écoute
	// accept et recv
	void pollLoop();
};

#endif