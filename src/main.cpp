#include <iostream>
#include <cstdlib>
#include "Server.hpp"
#include "ServerException.hpp"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    try
    {
        int port = std::atoi(argv[1]);
        Server server(port, argv[2]);
        server.start();
    }
    catch (const ServerException &e)
    {
        std::cerr << "Erreur serveur : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
