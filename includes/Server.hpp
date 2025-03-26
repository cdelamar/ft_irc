#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server
{
private:
	int _servSocket;
	int _port;
	std::string _password;

	void initSocket();

public:
	Server(int port, const std::string &password);
	Server(const Server &src);
    Server &operator=(const Server &src);
    ~Server();

	// accept et recv
    void start(); // test simple : afficher que le serveur écoute
};

#endif