#include "MessageTranslator.h"

// Use template for each Message for easy scalability. We expect more meesage and changes to happen in the future
template<typename T>
struct MessageTraits;

// Specializations
template<>
struct MessageTraits<JoinGameMessage> {
    // Call statics to enable compile time, so we don't need to instantiate the object and just use it as a container
    static constexpr std::string_view prefix = "JoinGame:"; // instiatiate it as pure compile time constant
    static std::string serialize(const JoinGameMessage& d) {
        return std::string(prefix) + d.playerName;
    }
    static Message deserialize(const std::string& payload) {
        std::string name = payload.substr(prefix.size());
        return { MessageType::JoinGame, JoinGameMessage{name} };
    }
};

template<>
struct MessageTraits<UpdateCycleMessage> {
    // Call statics to enable compile time, so we don't need to instantiate the object and just use it as a container
    static constexpr std::string_view prefix = "UpdateCycle:"; // instiatiate it as pure compile time constant

    static std::string serialize(const UpdateCycleMessage& d) {
        return std::string(prefix) + std::to_string(d.cycle);
    }
    static Message deserialize(const std::string& payload) {
        std::string num = payload.substr(prefix.size());
        int cycle = std::stoi(num);
        return { MessageType::UpdateCycle, UpdateCycleMessage{cycle} };
    }
};

template<>
struct MessageTraits<JoinLobbyMessage> {
    static constexpr std::string_view prefix = "JoinLobby:";

    static std::string serialize(const JoinLobbyMessage& d) {
        return std::string(prefix) + d.playerName;
    }

    static Message deserialize(const std::string& payload) {
        std::string name = payload.substr(prefix.size());
        return { MessageType::JoinLobby, JoinLobbyMessage{name}};
    }
};

template<>
struct MessageTraits<LeaveLobbyMessage>{
    static constexpr std::string_view prefix = "LeaveLobby:";
    static std::string serialize(const LeaveLobbyMessage& d) {
        return std::string(prefix) + d.playerName;
    }
    static Message deserialize(const std::string& payload) {
        std::string name = payload.substr(prefix.size());
        return { MessageType::LeaveLobby, LeaveLobbyMessage{name}};
    }
};

template<>
struct MessageTraits<LobbyStateMessage>{
    static constexpr std::string_view prefix = "LobbyState:";
    static std::string serialize(const LobbyStateMessage& d) {
        return std::string(prefix) + d.currentLobbyID;
    }
    static Message deserialize(const std::string& payload) {
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
    static Message deserialize(const std::string& payload) {
        std::string gameTypeStr = payload.substr(prefix.size());
        int gameTypeInt = gameTypeStr.empty() ? 0 : std::stoi(gameTypeStr);
        return { MessageType::BrowseLobbies, BrowseLobbiesMessage{static_cast<GameType>(gameTypeInt)} };
    }
};

template<>
struct MessageTraits<GetLobbyStateMessage>{
    static constexpr std::string_view prefix = "GetLobbyState:";
    static std::string serialize(const GetLobbyStateMessage& getLobbyStateMsg) {
        return std::string(prefix);
    }
    static Message deserialize(const std::string& payload) {
        return { MessageType::GetLobbyState, GetLobbyStateMessage{} };
    }
};

template<>
struct MessageTraits<ErrorMessage>{
    static constexpr std::string_view prefix = "Error:";
    static std::string serialize(const ErrorMessage& errorMsg) {
        return std::string(prefix) + errorMsg.reason;
    }
    static Message deserialize(const std::string& payload) {
        std::string reason = payload.substr(prefix.size());
        return { MessageType::Error, ErrorMessage{reason} };
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

Message MessageTranslator::deserialize(const std::string& payload)
{
    if (payload.starts_with(MessageTraits<JoinGameMessage>::prefix)) {
        return MessageTraits<JoinGameMessage>::deserialize(payload);
    }
    else if (payload.starts_with(MessageTraits<UpdateCycleMessage>::prefix)) {
        return MessageTraits<UpdateCycleMessage>::deserialize(payload);
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
    else {
        return { MessageType::Empty, {} };
    }
}