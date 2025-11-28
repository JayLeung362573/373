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

    /// Input requests from Server
    RequestTextInput,
    RequestChoiceInput,
    RequestRangeInput,

    /// Responses from Players
    ResponseTextInput,
    ResponseChoiceInput,
    ResponseRangeInput,

    /// game message/prompt from the game
    GameOutput,
    GameOver,
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
    int gameType = 0;
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

struct RequestTextInputMessage{
    std::string prompt;
};

struct RequestChoiceInputMessage {
    std::string prompt;
    std::vector<std::string> choices;
};

struct RequestRangeInputMessage {
    std::string prompt;
    int min;
    int max;
};

struct ResponseTextInputMessage {
    std::string input;
    std::string promptReference;
};

struct ResponseChoiceInputMessage {
    std::string choice;
    std::string promptRef;
};

struct ResponseRangeInputMessage {
    int value;
    std::string promptRef;
};

struct GameOutputMessage{
    std::string message;
};

struct GameOverMessage{
    std::string winner;
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
        ErrorMessage,

        RequestTextInputMessage,
        RequestChoiceInputMessage,
        RequestRangeInputMessage,

        ResponseTextInputMessage,
        ResponseChoiceInputMessage,
        ResponseRangeInputMessage,

        GameOutputMessage,
        GameOverMessage
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
