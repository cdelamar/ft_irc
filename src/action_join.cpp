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

void handleJoin(Server &srv, int fd, const Command &cmd)
{
	std::string chan;
	if (!parsingJoin(cmd, chan))
		return srv.sendToClient(fd, "461 JOIN :Need more parameters");

	if (!srv.channelExists(chan))
		srv.createChannel(chan);

	Channel &c = srv.getChannel(chan);
	int err = permChecker(c, fd, cmd);
	if (err)
		return srv.sendToClient(fd, cpp98_toString(err) + " " + chan + " :Cannot join channel");

	if (c.isMember(fd))
		return;

	Client &cli = srv.getClient(fd);
	c.addMember(cli);
	c.removeInvite(fd);

	sendJoinMsg(srv, c, cli);

	if (c.memberCount() == 1)
		c.promoteToOperator(fd);
}
