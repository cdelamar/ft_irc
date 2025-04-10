#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
	int _fd;
	std::string _msgBuffer;
    std::string _nickname;

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

    // getter setter nickname
    void setNickname(const std::string &nick);
    const std::string &getNickname() const;
};

#endif