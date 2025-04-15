#include "Client.hpp"

Client::Client()
    : _fd(-1), _msgBuffer(""), _nickname(""), _username(""), _realname(""), _isRegistered(false), _isPassSaved(false) {}

Client::Client(int fd)
    : _fd(fd), _msgBuffer(""), _nickname(""), _username(""), _realname(""), _isRegistered(false), _isPassSaved(false) {}

Client::Client(const Client &src)
    : _fd(src._fd),
      _msgBuffer(src._msgBuffer),
      _nickname(src._nickname),
      _username(src._username),
      _realname(src._realname),
      _isRegistered(src._isRegistered),
	  _isPassSaved(src._isPassSaved) {}

Client &Client::operator=(const Client &src)
{
    if (this != &src)
    {
        _fd = src._fd;
        _msgBuffer = src._msgBuffer;
        _nickname = src._nickname;
        _username = src._username;
        _realname = src._realname;
        _isRegistered = src._isRegistered;
        _isPassSaved = src._isPassSaved;
    }
    return *this;
}

Client::~Client() {}

/*
defnition d'une commande en IRC - RFC 2812
ligne de texte envoyee par un client :
<commande> [param1] [param2] ... \r\n

Exemples :
NICK Bob\r\n
USER bob 0 * :Bob l'utilisateur\r\n
PING :12345\r\n
PRIVMSG #general :Salut tout le monde\r\n

Chaque commande se termine par \r\n  et il faut les lire une par une, dans l’ordre, depuis un buffer TCP.
*/

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
	/*
	'npos' veut dire 'no position'
	si un find est egale a 'no position' c'est que find n'a rien trouve
	comme find() retounr size_t, on se sert de 'npos' un peu comme d'un -1 en C
	*/

	return (_msgBuffer.find("\r\n") != std::string::npos); // est-ce que \r\n existe dans la chaine
}


std::string Client::extractCommand()
{
	size_t pos = _msgBuffer.find("\r\n");
	if (pos == std::string::npos)
		return "";

	std::string command = _msgBuffer.substr(0, pos);
	_msgBuffer.erase(0, pos + 2); // pour effacer \r\n
	return command;
}


void Client::setNickname(const std::string &nickname) { _nickname = nickname; }
const std::string &Client::getNickname() const { return _nickname; }

void Client::setUsername(const std::string &username) { _username = username; }
const std::string &Client::getUsername() const { return _username; }

void Client::setRealname(const std::string &realname) { _realname = realname; }
const std::string &Client::getRealname() const { return _realname; }

bool Client::isRegistered() const { return _isRegistered; }
void Client::checkRegistered()
{
	/*
	RFC 2812 - section 3.1 "Connection Registration" :
    A client is registered once it has sent the NICK and USER commands...

	Note: en RFC 2812, 'realname' est acceptable meme vide
	il s'agit d'une donnee qui peut etre utile pour les autres utilisateurs,
	mais pas necessaire pour la machine
	*/

	if (!_username.empty() && !_nickname.empty())
		_isRegistered = true;
}

bool Client::isPassSaved() const { return _isPassSaved; }
void Client::setPassSaved(bool state) { _isPassSaved = state; }
