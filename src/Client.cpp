#include "Client.hpp"

Client::Client()
    : _fd(-1), _msgBuffer(""), _nickname("") {}

Client::Client(int fd)
    : _fd(fd), _msgBuffer(""), _nickname("") {}

Client::Client(const Client &src)
    : _fd(src._fd), _msgBuffer(src._msgBuffer), _nickname(src._nickname) {}

Client::~Client() {}
Client &Client::operator=(const Client &src)
{
    if (this != &src)
    {
        _fd = src._fd;
        _msgBuffer = src._msgBuffer;
    }
    return *this;
}


void Client::setNickname(const std::string &nick) { _nickname = nick; }
const std::string &Client::getNickname() const { return _nickname; }

int Client::getFd() const
{
	return _fd;
}

void Client::appendToBuffer(const std::string &data)
{
	_msgBuffer += data;
}

bool Client::hasCompleteCommand() const
{
	// 'npos' veut dire 'no position'
	// si un find est egale a 'no position' c'est que find n'a rien trouve
	// comme find() retounr size_t, on se sert de 'npos' un peu comme d'un -1 en C
	return (_msgBuffer.find("\r\n") != std::string::npos); // est-ce que \r\n existe dans la chaine
}

// defnition d'une commande en IRC - RFC 2812
//
// ligne de texte envoyee par un client

// <commande> [param1] [param2] ... \r\n

// Exemples :

// NICK Bob\r\n
// USER bob 0 * :Bob l'utilisateur\r\n
// PING :12345\r\n
// PRIVMSG #general :Salut tout le monde\r\n

// Chaque commande se termine par \r\n  et il faut les lire une par une, dans l’ordre, depuis un buffer TCP.

std::string Client::extractCommand()
{
	size_t pos = _msgBuffer.find("\r\n");
	if (pos == std::string::npos)
		return "";

	std::string command = _msgBuffer.substr(0, pos);
	_msgBuffer.erase(0, pos + 2); // pour effacer \r\n
	return command;
}