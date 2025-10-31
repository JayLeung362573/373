#pragma once

#include <string>

// TODO: Use tiny types?
using PlayerID = int;


struct Player
{
    PlayerID id;
    std::string name;
};
