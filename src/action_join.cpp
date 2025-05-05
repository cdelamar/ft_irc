#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "utils.hpp"

static bool parsingJoin(const Command &cmd, std::string &chan)
{
	if (cmd.params.empty())
		return false;
	chan = cmd.params[0];
	return true;
}

static bool join_i(Channel &c, int fd)
{
	return !c.isInviteOnly() || c.isInvited(fd);
}

static bool join_k(const Channel &c, const Command &cmd)
{
	if (!c.hasPassword())
		return true;
	if (cmd.params.size() < 2)
		return false;
	return cmd.params[1] == c.getPassword();
}

static bool join_l(const Channel &c)
{
	if (!c.hasUserLimit())
		return true;
	return c.memberCount() < c.getUserLimit();
}

static int permChecker(Channel &c, int fd, const Command &cmd)
{
	if (!join_i(c, fd))
        return 473;
	if (!join_k(c, cmd))
        return 475;
	if (!join_l(c))
        return 471;
	return 0;
}

static void sendJoinMsg(Server &srv, Channel &c, Client &cli)
{
	std::string ch = c.getName();
	std::string prefix = ":" + cli.getNickname() + "!" + cli.getUsername() + "@localhost";

	c.broadcast(srv, prefix + " JOIN :" + ch, -1);

	if (c.getTopic().empty())
		srv.sendToClient(cli.getFd(), "331 " + ch + " :No topic is set");
	else
		srv.sendToClient(cli.getFd(), "332 " + ch + " :" + c.getTopic());

	std::vector<int> members = c.getMemberFds();
	std::string names;

	for (size_t i = 0; i < members.size(); ++i)
	{
		Client &m = srv.getClient(members[i]);
		if (!names.empty()) names += " ";
		if (c.isOperator(members[i])) names += "@";
		names += m.getNickname();
	}

	srv.sendToClient(cli.getFd(), "353 " + cli.getNickname() + " = " + ch + " :" + names);
	srv.sendToClient(cli.getFd(), "366 " + cli.getNickname() + " " + ch + " :End of /NAMES");
}

static bool sendError(Server &srv, int fd, const std::string &chanName, int err)
{
	if (err == 473)
	{
		srv.sendToClient(fd, "473 " + chanName + " :Cannot join channel (+i)");
		return true;
	}
	else if (err == 475)
	{
		srv.sendToClient(fd, "475 " + chanName + " :Cannot join channel (+k)");
		return true;
	}
	else if (err == 471)
	{
		srv.sendToClient(fd, "471 " + chanName + " :Cannot join channel (+l)");
		return true;
	}

	return false;
}

void handleJoin(Server &srv, int fd, const Command &cmd)
{
	std::string chanName;
	if (!parsingJoin(cmd, chanName))
		return srv.sendToClient(fd, "461 JOIN :Need more parameters");

	if (!srv.channelExists(chanName))
		srv.createChannel(chanName);

	Channel &channel = srv.getChannel(chanName);
	if (channel.isMember(fd))
		return;

	int err = permChecker(channel, fd, cmd);
	if (sendError(srv, fd, chanName, err))
		return;

	Client &client = srv.getClient(fd);
	channel.addMember(client);
	channel.removeInvite(fd);

	sendJoinMsg(srv, channel, client);

	if (channel.memberCount() == 1)
		channel.promoteToOperator(fd);
}
