#ifndef SERVEREXCEPTION_HPP
#define SERVEREXCEPTION_HPP

#include <stdexcept>
#include <string>

class ServerException : public std::exception
{
public:
    ServerException(const std::string &msg);
    virtual ~ServerException() throw();
    virtual const char* what() const throw();

private:
    std::string _msg;
};

#endif