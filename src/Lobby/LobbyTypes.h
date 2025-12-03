#pragma once
#include <string>
#include <cstdint>

class Lobby;

using ClientID = uintptr_t;
using LobbyID = std::string;
using GameID = std::string;

enum class GameType{
    Default = 0,
    /// RockPaperScissor
    NumberBattle = 1,
    ChoiceBattle = 2
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

// Added explicit result/error reporting to the registry layer
enum class LobbyError{
    None = 0,
    AlreadyInLobby,
    LobbyNotFound,
    LobbyFull
};

struct LobbyResult{
    Lobby* lobby = nullptr;
    LobbyError error = LobbyError::None;

    bool succeeded() const { return error == LobbyError::None && lobby != nullptr; }
};
