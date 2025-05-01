#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

static std::string stringJoin(const std::vector<std::string> &v, size_t start)
{
	std::string res;
	for (size_t i = start; i < v.size(); ++i)
	{
		if (!res.empty())
			res += " ";
		res += v[i];
	}
	return res;
}

static bool parsingTopic(const Command &cmd, std::string &chan, std::string &topic)
{
	if (cmd.params.empty())
		return false;
	chan = cmd.params[0];
	if (cmd.params.size() >= 2)
		topic = stringJoin(cmd.params, 1);
	return true;
}

static bool topic_t(Channel &c, int fd)
{
	return !c.isTopicRestricted() || c.isOperator(fd);
}

static void replyTopic(Server &srv, int fd, const Channel &c)
{
	if (c.getTopic().empty())
		srv.sendToClient(fd, "331 " + c.getName() + " :No topic is set");
	else
		srv.sendToClient(fd, "332 " + c.getName() + " :" + c.getTopic());
}

static void broadcastTopic(Server &srv, const Channel &c, const Client &cli, const std::string &topic)
{
	std::string prefix = ":" + cli.getNickname() + "!" + cli.getUsername() + "@localhost";
	c.broadcast(srv, prefix + " TOPIC " + c.getName() + " :" + topic, -1);
}


void handleTopic(Server &srv, int fd, const Command &cmd)
{
	std::string chan;
	std::string topic;

	if (!parsingTopic(cmd, chan, topic))
		return srv.sendToClient(fd, "461 TOPIC :Need more parameters");

	if (!srv.channelExists(chan))
		return srv.sendToClient(fd, "403 " + chan + " :No such channel");

	Channel &c = srv.getChannel(chan);
	if (!c.isMember(fd))
		return srv.sendToClient(fd, "442 " + chan + " :You're not on that channel");

	if (topic.empty())
		return replyTopic(srv, fd, c);

	if (!topic_t(c, fd))
		return srv.sendToClient(fd, "482 " + chan + " :You're not channel operator");

	c.setTopic(topic);
	broadcastTopic(srv, c, srv.getClient(fd), topic);
}
