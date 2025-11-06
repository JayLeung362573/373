#include "MessageTranslator.h"

// Use template for each Meesage for easy scalability. We expect more meesage and changes to happen in the future
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
}
;
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
    
    else {
        return { MessageType::Empty, {} };
    }
}