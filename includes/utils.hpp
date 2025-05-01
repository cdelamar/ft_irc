#pragma once

#include "Command.hpp"
#include "CommandHandler.hpp"
#include "Server.hpp"

/*
RFC 2812 – Rappel sur les nicknames :

    Commencent obligatoirement par une lettre (A-Z ou a-z)
    Peuvent contenir lettres, chiffres, - / _
    les maj/minuscule comptent
*/


std::string cpp98_toString (int n);
bool isValidNickname(const std::string &nickname);
