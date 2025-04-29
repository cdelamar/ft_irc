#include <vector>
#include <map>
#include <string>
#include <ctime>
#include <iostream>
#include <sstream>

// creer une base utilisateur User

struct User {
	int id;
	std::string name;
};

void searchVector( const std::vector<User> &users, int findID)
{
	size_t i = 0;

	while (i != users.size())
	{
		if (users[i].id == findID)
		{
			std::cout << "VECTOR : found " << users[i].id << std::endl;
			return;
		}
		i++;
	}

	std::cout << "pas trouve" << std::endl;
	return;
}

void searchMap( const std::map<int, User> &users, int findID)
{
	std::map<int, User>::const_iterator it = users.find(findID);
    if (it != users.end())
    {
        std::cout << "[map] Found: " << it->second.name << " (id=" << it->first << ")" << std::endl;
    }
    else
    {
        std::cout << "[map] Not found id=" << findID << std::endl;
    }
}

int main()
{
	std::vector<User> userVector;
	std::map<int, User> userMap;

	for (int i = 1; i < 10; i++)
	{
		User user;
		user.id = i;
		std::ostringstream oss;
		oss<< "user" << i ;
		user.name = oss.str();

		userVector.push_back(user);
		userMap[i] = user;
	}

	searchMap(userMap, 4);
	searchVector(userVector, 4);

}