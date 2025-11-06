#pragma once
#include <string>
#include <cstdint>

using ClientID = uintptr_t;
using LobbyID = std::string;
using GameID = std::string;

enum class GameType{
    Default,
    /// RockPaperScissor
};

enum class LobbyState{
    Open,
    InGame,
    GameOver,
};

struct LobbyInfo{
    LobbyID lobbyID;
    std::string lobbyName;
    GameType gameType;
    ClientID hostID;
    size_t currentPlayers;
    size_t maxPlayers;
    size_t minPlayers;
    LobbyState state;
};