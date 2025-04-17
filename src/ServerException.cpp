#include "ServerException.hpp"

ServerException::ServerException(const std::string &msg) : _msg(msg) {}
ServerException::~ServerException() throw() {}

const char* ServerException::what() const throw()
{
    return _msg.c_str();
}
