#pragma once

#include <memory>
#include <variant>
#include <string>
#include <cstdint>
#include <utility>
#include "Lobby.h"

enum class MessageType : uint8_t
{
    Empty = 0,
    JoinGame,
    UpdateCycle,

    /// Lobby messages
    JoinLobby,
};

struct JoinGameMessage
{
    std::string playerName;
};

struct UpdateCycleMessage
{
    int cycle;
};

struct JoinLobbyMessage{
    std::string playerName;
};

using MessageData = std::variant<
        std::monostate,
        JoinGameMessage,
        UpdateCycleMessage,
        JoinLobbyMessage
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
