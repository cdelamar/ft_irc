#ifndef COMMAND_HPP
#define COMMAND_HPP

// TODO : voir si je peux pas decaler ca autre part,
// sinon le documenter pour expliquer comment ca marche

#include <string>
#include <vector>

struct Command {
    std::string name;                  // ex: "NICK"
    std::vector<std::string> params;  // ["Bob"]
    std::string raw;                  // "NICK Bob"

	Command() : name(""), params(), raw("") {}
};

#endif