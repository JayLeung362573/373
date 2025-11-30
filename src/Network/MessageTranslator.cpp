#include "MessageTranslator.h"
#include <string_view>
#include <string>
#include <stdexcept>
#include <iostream>

/// to parse string_view to int, for lobbyID and gameType
static int parseInt(std::string_view sv) {
    try {
        return std::stoi(std::string(sv));
    }
    catch (const std::exception&) {
        return 0;
    }
}

// Use template for each Message for easy scalability. We expect more message and changes to happen in the future
template<typename T>
struct MessageTraits;

// Specializations
template<>
struct MessageTraits<StartGameMessage> {
    // Call statics to enable compile time, so we don't need to instantiate the object and just use it as a container
    static constexpr std::string_view prefix = "StartGame:"; // instantiate it as pure compile time constant
    static std::string serialize(const StartGameMessage& d) {
        return std::string(prefix) + d.playerName;
    }
    static Message deserialize(const std::string_view payload) {
        std::string_view name = payload.substr(prefix.size());
        return { MessageType::StartGame, StartGameMessage{std::string(name)} };
    }
};

template<>
struct MessageTraits<UpdateCycleMessage> {
    // Call statics to enable compile time, so we don't need to instantiate the object and just use it as a container
    static constexpr std::string_view prefix = "UpdateCycle:"; // instantiate it as pure compile time constant

    static std::string serialize(const UpdateCycleMessage& d) {
        return std::string(prefix) + std::to_string(d.cycle);
    }
    static Message deserialize(std::string_view payload) {
        auto num = payload.substr(prefix.size());
        int cycle = parseInt(num);
        return { MessageType::UpdateCycle, UpdateCycleMessage{cycle} };
    }
};

template<>
struct MessageTraits<CreateLobbyMessage> {
    static constexpr std::string_view prefix = "CreateLobby";

    static std::string serialize(const CreateLobbyMessage&) {
        return std::string(prefix);
    }

    static Message deserialize(std::string_view) {
        return { MessageType::CreateLobby, CreateLobbyMessage{} };
    }
};

template<>
struct MessageTraits<StartJoinLobbyMessage> {
    static constexpr std::string_view prefix = "JoinLobby";

    static std::string serialize(const StartJoinLobbyMessage&) { return std::string(prefix); }
    static Message deserialize(std::string_view) {
        return { MessageType::StartJoinLobby, StartJoinLobbyMessage{} };
    }
};

/// Message format: JoinLobby:PlayerName|LobbyName|GameType
template<>
struct MessageTraits<JoinLobbyMessage> {
    static constexpr std::string_view prefix = "InternalJoinLobby:";

    static std::string serialize(const JoinLobbyMessage& d) {
        return std::string(prefix) + d.playerName + "|" + d.lobbyName + "|" + std::to_string(d.gameType);
    }

    static Message deserialize(std::string_view payload) {
        auto content = payload.substr(prefix.size());
        size_t firstDelimiter = content.find('|');
        /// split PlayerName|LobbyName|GameType
        std::string_view playerName;
        std::string_view lobbyName;
        int type = 0;

        /// PlayerName only
        if(firstDelimiter == std::string_view::npos){
            playerName = content;
        }
        else{
            /// PlayerName|...
            playerName = content.substr(0, firstDelimiter);

            size_t secondDelimiter = content.find('|', firstDelimiter + 1);

            if(secondDelimiter == std::string_view ::npos){
                lobbyName = content.substr(firstDelimiter + 1);
            }
            else{
                /// PlayerName|LobbyName|GameType
                lobbyName = content.substr(firstDelimiter + 1, secondDelimiter -(firstDelimiter + 1));

                std::string_view typeStr = content.substr(secondDelimiter + 1);
                if (!typeStr.empty()) {
                    try {
                        type = parseInt(typeStr); // Convert string to int
                    } catch (...) {
                        type = 0; // Fallback on error
                    }
                }
            }
        }
        return {MessageType::JoinLobby, JoinLobbyMessage{std::string(playerName), std::string(lobbyName), type}};
    }
};

template<>
struct MessageTraits<LeaveLobbyMessage>{
    static constexpr std::string_view prefix = "LeaveLobby:";
    static std::string serialize(const LeaveLobbyMessage& d) {
        return std::string(prefix) + d.playerName;
    }
    static Message deserialize(const std::string_view payload) {
        return { MessageType::LeaveLobby, LeaveLobbyMessage{std::string(payload.substr(prefix.size()))}};
    }
};

template<>
struct MessageTraits<LobbyStateMessage>{
    static constexpr std::string_view prefix = "LobbyState:";
    static std::string serialize(const LobbyStateMessage& d) {
        return std::string(prefix) + d.currentLobbyID;
    }
    static Message deserialize(const std::string_view payload) {
        LobbyStateMessage msg;
        msg.currentLobbyID = "";
        return { MessageType::LobbyState, msg };
    }
};

template<>
struct MessageTraits<BrowseLobbiesMessage> {
    static constexpr std::string_view prefix = "BrowseLobbies:";
    static std::string serialize(const BrowseLobbiesMessage& d) {
        return std::string(prefix) + std::to_string(static_cast<int>(d.gameType));
    }
    static Message deserialize(const std::string_view payload) {
        std::string_view gameTypeStr = payload.substr(prefix.size());
        int gameTypeInt = gameTypeStr.empty() ? 0 : parseInt(gameTypeStr);
        return { MessageType::BrowseLobbies, BrowseLobbiesMessage{static_cast<GameType>(gameTypeInt)} };
    }
};

template<>
struct MessageTraits<GetLobbyStateMessage>{
    static constexpr std::string_view prefix = "GetLobbyState:";
    static std::string serialize(const GetLobbyStateMessage& getLobbyStateMsg) {
        return std::string(prefix);
    }
    static Message deserialize(const std::string_view payload) {
        return { MessageType::GetLobbyState, GetLobbyStateMessage{} };
    }
};

template<>
struct MessageTraits<RequestTextInputMessage> {
    static constexpr std::string_view prefix = "RequestTextInput:";

    static std::string serialize(const RequestTextInputMessage& requestTextInputMsg) {
        return std::string(prefix) + requestTextInputMsg.prompt;
    }

    static Message deserialize(const std::string_view payload) {
        std::string_view prompt = payload.substr(prefix.size());
        return { MessageType::RequestTextInput, RequestTextInputMessage{std::string(prompt)} };
    }
};

template<>
struct MessageTraits<RequestChoiceInputMessage> {
    static constexpr std::string_view prefix = "RequestChoiceInput:";

    static std::string serialize(const RequestChoiceInputMessage& requestChoiceInputMsg) {
        return std::string(prefix) + requestChoiceInputMsg.prompt;
    }

    static Message deserialize(const std::string_view payload) {
        std::string_view prompt = payload.substr(prefix.size());
        return { MessageType::RequestChoiceInput, RequestChoiceInputMessage{std::string(prompt)} };
    }
};

template<>
struct MessageTraits<RequestRangeInputMessage> {
    static constexpr std::string_view prefix = "RequestRangeInput:";

    static std::string serialize(const RequestRangeInputMessage& requestRangeInputMsg) {
        return std::string(prefix) + requestRangeInputMsg.prompt
        + "|" + std::to_string(requestRangeInputMsg.min)
        + "|" + std::to_string(requestRangeInputMsg.max);
    }

    static Message deserialize(const std::string_view payload) {
        std::string_view prompt = payload.substr(prefix.size());
        return { MessageType::RequestRangeInput, RequestRangeInputMessage{std::string(prompt)} };
    }
};

template<>
struct MessageTraits<ResponseTextInputMessage> {
    static constexpr std::string_view prefix = "ResponseTextInput:";

    static std::string serialize(const ResponseTextInputMessage& responseTextInputMsg) {
        return std::string(prefix) + responseTextInputMsg.input +
        "|" + responseTextInputMsg.promptReference;
    }

    static Message deserialize(const std::string_view payload) {
        std::string_view content = payload.substr(prefix.size());
        size_t delimiter = content.find('|');
        return { MessageType::ResponseTextInput,
                 ResponseTextInputMessage{std::string(content.substr(0, delimiter)),
                                          std::string (content.substr(delimiter + 1))} };
    }
};

template<>
struct MessageTraits<ResponseChoiceInputMessage> {
    static constexpr std::string_view prefix = "ResponseChoiceInput:";

    static std::string serialize(const ResponseChoiceInputMessage& responseChoiceInputMsg) {
        return std::string(prefix) + responseChoiceInputMsg.choice + "|" + responseChoiceInputMsg.promptRef;
    }

    static Message deserialize(const std::string_view payload) {
        std::string_view prompt = payload.substr(prefix.size());
        size_t delimiter = prompt.find('|');

        if (delimiter == std::string::npos) {
            return { MessageType::Empty, {} };
        }

        return { MessageType::ResponseChoiceInput,
                 ResponseChoiceInputMessage{
            std::string(prompt.substr(0, delimiter)),
            std::string (prompt.substr(delimiter + 1))}};
    }
};

template<>
struct MessageTraits<ResponseRangeInputMessage> {
    static constexpr std::string_view prefix = "ResponseRangeInput:";

    static std::string serialize(const ResponseRangeInputMessage& responseRangeInputMsg) {
        return std::string(prefix) + std::to_string(responseRangeInputMsg.value)
        + "|" + responseRangeInputMsg.promptRef;
    }

    static Message deserialize(const std::string_view payload) {
        std::string_view prompt = payload.substr(prefix.size());
        size_t delimiter = prompt.find('|');
        int val = parseInt(prompt.substr(0, delimiter));
        return { MessageType::ResponseRangeInput,
                 ResponseRangeInputMessage{val, std::string(prompt.substr(delimiter + 1))} };
        }
};

template<>
struct MessageTraits<GameOutputMessage> {
    static constexpr std::string_view prefix = "GameOutput:";

    static std::string serialize(const GameOutputMessage& gameOutputMsg) {
        return std::string(prefix) + gameOutputMsg.message;
    }

    static Message deserialize(const std::string_view payload) {
        std::string_view msg = payload.substr(prefix.size());
        return { MessageType::GameOutput, GameOutputMessage{std::string (msg)} };
    }
};

template<>
struct MessageTraits<GameOverMessage> {
    static constexpr std::string_view prefix = "GameOver:";

    static std::string serialize(const GameOverMessage& gameOverMsg) {
        return std::string(prefix) + gameOverMsg.winner;
    }

    static Message deserialize(const std::string_view payload) {
        std::string_view winner = payload.substr(prefix.size());
        return { MessageType::GameOver, GameOverMessage{std::string(winner)} };
    }
};

template<>
struct MessageTraits<ErrorMessage>{
    static constexpr std::string_view prefix = "Error:";
    static std::string serialize(const ErrorMessage& errorMsg) {
        return std::string(prefix) + errorMsg.reason;
    }
    static Message deserialize(const std::string_view payload) {
        std::string_view reason = payload.substr(prefix.size());
        return { MessageType::Error, ErrorMessage{std::string(reason)} };
    }
};

std::string MessageTranslator::serialize(const Message& msg)
{
    // using std::visit to access std::variant that MessageData contained
    std::string serializedMessage = std::visit(
        [](auto&& data) -> std::string {
            using T = std::decay_t<decltype(data)>; // Access the variant type
            if constexpr (!std::is_same_v<T, std::monostate>) {
                return MessageTraits<T>::serialize(data);
            }
            else {
                return "Empty";
            }
        }, msg.data);
    
    return serializedMessage;
}

Message MessageTranslator::deserialize(std::string_view payload)
{
    std::cout << "[Translator] Checking payload: '" << payload << "'\n";
    if (payload.starts_with(MessageTraits<StartGameMessage>::prefix)) {
        return MessageTraits<StartGameMessage>::deserialize(payload);
    }
    else if (payload.starts_with(MessageTraits<UpdateCycleMessage>::prefix)) {
        return MessageTraits<UpdateCycleMessage>::deserialize(payload);
    }
    else if (payload.starts_with(MessageTraits<CreateLobbyMessage>::prefix)) {
        std::cout << "[Translator] Found CreateLobbyMessage!\n";
        return MessageTraits<CreateLobbyMessage>::deserialize(payload);
    }
    else if(payload.starts_with(MessageTraits<StartJoinLobbyMessage>::prefix)) {
        return MessageTraits<StartJoinLobbyMessage>::deserialize(payload);
    }
    else if(payload.starts_with(MessageTraits<JoinLobbyMessage>::prefix)) {
        return MessageTraits<JoinLobbyMessage>::deserialize(payload);
    }
    else if(payload.starts_with(MessageTraits<LeaveLobbyMessage>::prefix)) {
        return MessageTraits<LeaveLobbyMessage>::deserialize(payload);
    }
    else if(payload.starts_with(MessageTraits<LobbyStateMessage>::prefix)) {
        return MessageTraits<LobbyStateMessage>::deserialize(payload);
    }
    else if(payload.starts_with(MessageTraits<BrowseLobbiesMessage>::prefix)) {
        return MessageTraits<BrowseLobbiesMessage>::deserialize(payload);
    }
    else if(payload.starts_with(MessageTraits<GetLobbyStateMessage>::prefix)) {
        return MessageTraits<GetLobbyStateMessage>::deserialize(payload);
    }
    else if(payload.starts_with(MessageTraits<ErrorMessage>::prefix)) {
        return MessageTraits<ErrorMessage>::deserialize(payload);
    }
    /// Game Request
    else if (payload.starts_with(MessageTraits<RequestTextInputMessage>::prefix)) {
        return MessageTraits<RequestTextInputMessage>::deserialize(payload);
    }
    else if (payload.starts_with(MessageTraits<RequestChoiceInputMessage>::prefix)) {
        return MessageTraits<RequestChoiceInputMessage>::deserialize(payload);
    }
    else if (payload.starts_with(MessageTraits<RequestRangeInputMessage>::prefix)) {
        return MessageTraits<RequestRangeInputMessage>::deserialize(payload);
    }
    /// game response
    else if (payload.starts_with(MessageTraits<ResponseTextInputMessage>::prefix)) {
        return MessageTraits<ResponseTextInputMessage>::deserialize(payload);
    }
    else if (payload.starts_with(MessageTraits<ResponseChoiceInputMessage>::prefix)) {
        return MessageTraits<ResponseChoiceInputMessage>::deserialize(payload);
    }
    else if (payload.starts_with(MessageTraits<ResponseRangeInputMessage>::prefix)) {
        return MessageTraits<ResponseRangeInputMessage>::deserialize(payload);
    }
    /// game output and game over message
    else if (payload.starts_with(MessageTraits<GameOutputMessage>::prefix)) {
        return MessageTraits<GameOutputMessage>::deserialize(payload);
    }
    else if (payload.starts_with(MessageTraits<GameOverMessage>::prefix)) {
        return MessageTraits<GameOverMessage>::deserialize(payload);
    }
    else {
        std::cout << "[Translator] No matching prefix found. Returning Empty.\n";
        return { MessageType::Empty, {} };
    }
}