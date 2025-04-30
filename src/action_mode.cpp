#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "Command.hpp"


static bool mod_i(Channel &chan, bool add)
{
    chan.setInviteOnly(add);
    return true;
}

static bool mod_t(Channel &chan, bool add)
{
    chan.setTopicRestricted(add);
    return true;
}

static bool mod_k(Server &serv, int fd, Channel &c, const Command &cmd, size_t &i, bool add) {
    if (add)
	{
        if (i >= cmd.params.size())
		{
            serv.sendToClient(fd, "461 MODE +k :Missing parameter");
            return false;
        }
        c.setPassword(cmd.params[i++]);
    }
	else
        c.clearPassword();
    return true;
}

static bool mod_l(Server &serv, int fd, Channel &c, const Command &cmd, size_t &i, bool add)
{
    if (add)
	{
        if (i >= cmd.params.size())
		{
            serv.sendToClient(fd, "461 MODE +l :Missing parameter");
            return false;
        }
        c.setUserLimit(std::atoi(cmd.params[i++].c_str()));
    }
	else
        c.clearUserLimit();
    return true;
}

static bool unknown(Server &serv, int fd, char flag)
{
    serv.sendToClient(fd, "472 " + std::string(1, flag) + " :uknown mode flag");
    return true;
}

static bool parsingMode(Server &serv, int fd, Channel &c, const Command &cmd) {
    std::string mod = cmd.params[1];
    bool add = true;
    size_t i = 2;
    size_t pos = 0;

    while (pos < mod.size())
	{
        char flag = mod[pos];

        if (flag == '+') { add = true; ++pos; continue; }
        if (flag == '-') { add = false; ++pos; continue; } // joli ca, fin jtrouve

        bool ok = true;

        if (flag == 'i')
			ok = mod_i(c, add);
        else if (flag == 't')
			ok = mod_t(c, add);
        else if (flag == 'k')
			ok = mod_k(serv, fd, c, cmd, i, add);
        else if (flag == 'l')
			ok = mod_l(serv, fd, c, cmd, i, add);
        else ok = unknown(serv, fd, flag);

        if (!ok) return false;
        ++pos;
    }

    return true;
}

// handleMode : commande IRC MODE
void handleMode(Server &server, int clientFd, const Command &cmd)
{
    if (cmd.params.empty())
        return server.sendToClient(clientFd, "461 MODE :Missing parameters");

    std::string ch = cmd.params[0];
    if (!server.channelExists(ch))
        return server.sendToClient(clientFd, "403 " + ch + " :No such channel");

    Channel &c = server.getChannel(ch);

    if (cmd.params.size() == 1)
        return server.sendToClient(clientFd, "324 " + ch + " " + c.strModes());

    if (!c.isOperator(clientFd))
        return server.sendToClient(clientFd, "482 " + ch + " :You're not channel operator");

    if (!parsingMode(server, clientFd, c, cmd))
        return;

    server.sendToClient(clientFd, "324 " + ch + " " + c.strModes());
}
