#ifndef COMMAND_HANDLERS_HPP
#define COMMAND_HANDLERS_HPP

#include "Command.hpp"
#include "Server.hpp"


    //    431    ERR_NONICKNAMEGIVEN
    //           ":No nickname given"

    //      - Returned when a nickname parameter expected for a
    //        command and isn't found.

    //    432    ERR_ERRONEUSNICKNAME
    //           "<nick> :Erroneous nickname"

    //      - Returned after receiving a NICK message which contains
    //        characters which do not fall in the defined set.  See
    //        section 2.3.1 for details on valid nicknames.

    //    433    ERR_NICKNAMEINUSE
    //           "<nick> :Nickname is already in use"

    //      - Returned when a NICK message is processed that results
    //        in an attempt to change to a currently existing
    //        nickname.

    //    461    ERR_NEEDMOREPARAMS
    //           "<command> :Not enough parameters"

    //      - Returned by the server by numerous commands to
    //        indicate to the client that it didn't supply enough
    //        parameters.

    //    462    ERR_ALREADYREGISTRED
    //           ":Unauthorized command (already registered)"

    //      - Returned by the server to any link which tries to
    //        change part of the registered details (such as
    //        password or user details from second USER message).

    //    463    ERR_NOPERMFORHOST
    //           ":Your host isn't among the privileged"

    //      - Returned to a client which attempts to register with
    //        a server which does not been setup to allow
    //        connections from the host the attempted connection
    //        is tried.

    //    464    ERR_PASSWDMISMATCH
    //           ":Password incorrect"

    //      - Returned to indicate a failed attempt at registering
    //        a connection for which a password was required and
    //        was either not given or incorrect.

#define ERR_NONICKNAMEGIVEN    "431"
#define ERR_ERRONEUSNICKNAME   "432"
#define ERR_NICKNAMEINUSE      "433"
#define ERR_NEEDMOREPARAMS     "461"
#define ERR_ALREADYREGISTRED   "462"
#define ERR_PASSWDMISMATCH     "464"

// Handler pour la commande NICK
void handleNick(Server &server, int clientFd, const Command &cmd);
void handleUser(Server &server, int clientFd, const Command &cmd);
void handlePass(Server &server, int clientFd, const Command &cmd);
void handleJoin(Server &server, int clientFd, const Command &cmd);
void handlePrivMsg(Server &server, int clientFd, const Command &cmd);

#endif
