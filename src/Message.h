#pragma once

#include <memory>
#include <variant>
#include <string>
#include <cstdint>
#include <utility>
#include "Lobby/Lobby.h"

enum class MessageType : uint8_t
{
    Empty = 0,
    StartGame,
    UpdateCycle,

    /// Lobby messages
    JoinLobby,
    LeaveLobby,
    LobbyState,
    BrowseLobbies,
    GetLobbyState, // client requests the current lobby state
    Error,
};

struct StartGameMessage
{
    std::string playerName;
};

struct UpdateCycleMessage
{
    int cycle;
};

struct JoinLobbyMessage{
    std::string playerName;
    std::string lobbyName;
};

struct LeaveLobbyMessage{
    std::string playerName;
};

struct LobbyStateMessage{
    std::vector<LobbyInfo> lobbies;
    std::string currentLobbyID;
};

struct BrowseLobbiesMessage{
    GameType gameType = GameType::Default;
};

struct GetLobbyStateMessage{

};

struct ErrorMessage{
    std::string reason;
};

using MessageData = std::variant<
        std::monostate,
        StartGameMessage,
        UpdateCycleMessage,

        JoinLobbyMessage,
        LeaveLobbyMessage,
        LobbyStateMessage,
        BrowseLobbiesMessage,
        GetLobbyStateMessage,
        ErrorMessage
        >;

struct Message
{
    MessageType type = MessageType::Empty;
    MessageData data;
};

struct ClientMessage
{
    uintptr_t clientID;
    Message message;
};
