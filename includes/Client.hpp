#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
	int         _fd;
	std::string _msgBuffer;
    std::string _nickname;
    std::string _username;
    std::string _realname;
    bool        _isRegistered;
    bool        _isPassSaved;

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

    // Getter / Setter nickname
    void setNickname(const std::string &nickname);
    const std::string &getNickname() const;

    // Getter / Setter username
    void setUsername(const std::string &username);
    const std::string &getUsername() const;

    // Getter / Setter realName
    void setRealname(const std::string &realname);
    const std::string &getRealname() const;

    // inscription au registre IRC
    bool isRegistered() const;
    void checkRegistered();

    // verfication du mot de passe enrgistre par le client
    bool isPassSaved() const;
    void setPassSaved(bool state);
};

#endif