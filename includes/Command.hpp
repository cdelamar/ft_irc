#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>

struct Command {
    std::string name;                  // ex: "NICK"
    std::vector<std::string> params;  // ["Bob"]
    std::string raw;                  // "NICK Bob"

	Command() : name(""), params(), raw("") {}
};

#endif