#include "utils.hpp"

std::string cpp98_toString (int n)
{
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

bool isValidNickname(const std::string &nickname)
{
	if (nickname.empty())
		return false;

	if (!std::isalpha(nickname[0]))
		return false;

	size_t i = 1;

	while(i < nickname.size())
	{
		if (!std::isalnum(nickname[i]) && nickname[i] != '-' && nickname[i] != '_')
			return false;
		i++;
	}

	return true;
}