#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "Command.hpp"


static bool mode_i(Channel &c, bool add)
{
    c.setInviteOnly(add);
    return true;
}

static bool mode_t(Channel &c, bool add)
{
    c.setTopicRestricted(add);
    return true;
}

static bool mode_k(Server &srv, int fd, Channel &c, const Command &cmd, size_t &i, bool add) {
    if (add)
	{
        if (i >= cmd.params.size())
		{
            srv.sendToClient(fd, "461 MODE +k :Missing parameter");
            return false;
        }
        c.setPassword(cmd.params[i++]);
    }
	else
        c.clearPassword();
    return true;
}

static bool mode_l(Server &srv, int fd, Channel &c, const Command &cmd, size_t &i, bool add)
{
    if (add)
	{
        if (i >= cmd.params.size())
		{
            srv.sendToClient(fd, "461 MODE +l :Missing parameter");
            return false;
        }
        c.setUserLimit(std::atoi(cmd.params[i++].c_str()));
    }
	else
        c.clearUserLimit();
    return true;
}

static bool unknown(Server &srv, int fd, char flag)
{
    srv.sendToClient(fd, "472 " + std::string(1, flag) + " :uknown mode flag");
    return true;
}

static bool parsingMode(Server &srv, int fd, Channel &c, const Command &cmd) {
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

        if (flag == 'i') ok = mode_i(c, add);
        else if (flag == 't') ok = mode_t(c, add);
        else if (flag == 'k') ok = mode_k(srv, fd, c, cmd, i, add);
        else if (flag == 'l') ok = mode_l(srv, fd, c, cmd, i, add);
        else ok = unknown(srv, fd, flag);

        if (!ok) return false;
        ++pos;
    }

    return true;
}

// handleMode : commande IRC MODE
void handleMode(Server &srv, int fd, const Command &cmd)
{
    if (cmd.params.empty())
        return srv.sendToClient(fd, "461 MODE :Missing parameters");

    std::string ch = cmd.params[0];
    if (!srv.channelExists(ch))
        return srv.sendToClient(fd, "403 " + ch + " :No such channel");

    Channel &c = srv.getChannel(ch);

    if (cmd.params.size() == 1)
        return srv.sendToClient(fd, "324 " + ch + " " + c.strModes());

    if (!c.isOperator(fd))
        return srv.sendToClient(fd, "482 " + ch + " :You're not channel operator");

    if (!parsingMode(srv, fd, c, cmd))
        return;

    srv.sendToClient(fd, "324 " + ch + " " + c.strModes());
}
