#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
	int _fd;
	std::string _msgBuffer;

public:
    Client();
    Client(int fd);
    Client(const Client &src);
    Client &operator=(const Client &src);
    ~Client();

	int getFd() const;

    void appendToBuffer(const std::string &data);
    bool hasCompleteCommand() const;
    std::string extractCommand();
};

#endif