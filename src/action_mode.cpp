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
        if (flag == '-') { add = false; ++pos; continue; }

        bool ok = true;

        if (flag == 'i')
            ok = mod_i(c, add);
        else if (flag == 't')
            ok = mod_t(c, add);
        else if (flag == 'k')
            ok = mod_k(serv, fd, c, cmd, i, add);
        else if (flag == 'l')
            ok = mod_l(serv, fd, c, cmd, i, add);
        else if (flag == 'o')
        {
            if (i >= cmd.params.size())
            {
                serv.sendToClient(fd, "461 MODE +o :Missing parameter");
                return false;
            }

            const std::string &targetNick = cmd.params[i++];
            std::map<int, Client> &clients = serv.getClients();
            int targetFd = -1;

            for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
            {
                if (it->second.getNickname() == targetNick && c.isMember(it->first))
                {
                    targetFd = it->first;
                    break;
                }
            }

            if (targetFd == -1)
            {
                serv.sendToClient(fd, "441 " + targetNick + " " + c.getName() + " :They aren't on that channel");
                return false;
            }

            if (add)
                c.promoteToOperator(targetFd);
            else
                c.demoteFromOperator(targetFd);

            // Broadcast mode change
            Client &source = serv.getClient(fd);
            std::string prefix = source.getNickname() + "!" + source.getUsername() + "@" + serv.getHostname();
            std::string msg = ":" + prefix + " MODE " + c.getName() + " " + (add ? "+o " : "-o ") + targetNick;
            c.broadcast(serv, msg, -1);

            ++pos; // continuer à parser les autres flags
            continue;
        }
        else
        {
            ok = unknown(serv, fd, flag);
        }

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

    if (cmd.params[0][0] != '#')
	    return;

    if (!server.channelExists(ch))
        return server.sendToClient(clientFd, "c'est la");

    Channel &c = server.getChannel(ch);

    if (cmd.params.size() == 1)
        return server.sendToClient(clientFd, "324 " + ch + " " + c.strModes());

    if (!c.isOperator(clientFd))
        return server.sendToClient(clientFd, "482 " + ch + " :You're not channel operator");

    if (!parsingMode(server, clientFd, c, cmd))
        return;

    server.sendToClient(clientFd, "324 " + ch + " " + c.strModes());
}
