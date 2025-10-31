#pragma once

#include <memory>
#include <variant>
#include <string>
#include <utility>

enum class MessageType : uint8_t
{
    Empty = 0,
    JoinGame,
    UpdateCycle,
};

struct JoinGameMessage
{
    std::string playerName;
};

struct UpdateCycleMessage{
    int cycle;
};

using MessageData = std::variant<std::monostate, JoinGameMessage, UpdateCycleMessage>;
struct Message
{
    MessageType type = MessageType::Empty;
    MessageData data;
};
