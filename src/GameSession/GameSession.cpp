#include "GameSession.h"
#include <iostream>
#include <utility>

GameSession::GameSession(LobbyID lobbyID, ast::GameRules rules, std::vector<LobbyMember> players)
    : m_lobbyID(std::move(lobbyID))
    , m_players(std::move(players))
    , m_interpreter(m_inputManager, convertRulesToProgram(rules))
    {

    std::cout << "[GameSession] Created session for lobby " << m_lobbyID
              << " with " << m_players.size() << " players\n";}

uintptr_t getClientID(const std::string& playerID, const std::vector<LobbyMember>& players) {
    for (const auto& p : players) {
        if (p.name == playerID) return p.clientID;
    }
    return 0;
}

std::vector<ClientMessage>
GameSession::start() {
    std::cout << "[GameSession] Starting game execution\n";

    m_interpreter.execute();

    return collectOutgoingMessages();
}

// TODO: Revisit once IO implemented in GameInterpreter
 std::vector<ClientMessage>
 GameSession::tick(const std::vector<ClientMessage>& incomingMessages){
    // 1. Process inputs from clients (ResponseTextInput, ResponseChoiceInput, etc.)
    processIncomingMessages(incomingMessages);

    // 2. Run the game logic
    m_interpreter.execute();

    return collectOutgoingMessages();
 }

bool
GameSession::isFinished() const {
    return !m_interpreter.needsIO();
}

std::optional<Program>
GameSession::convertRulesToProgram(ast::GameRules& rules) {
    Program program;
    program.statements = std::move(rules.statements);
    return std::make_optional(std::move(program));
}


void
GameSession::processIncomingMessages(const std::vector<ClientMessage> &messages) {
    std::vector<GameMessage> gameMessages;

    for (const auto &clientMsg: messages) {
        bool isPlayerInSession = false;
        for (const auto& player : m_players) {
            if (player.clientID == clientMsg.clientID) {
                isPlayerInSession = true;
                break;
            }
        }

        if (!isPlayerInSession) {
            continue;
        }

        auto converted = convertMessageToGameMessage(clientMsg);
        if (converted) {
            gameMessages.push_back(*converted);
        }
    }

    if (!gameMessages.empty()) {
        m_inputManager.handleIncomingMessages(gameMessages);
        std::cout << "[GameSession] Processed " << gameMessages.size()
                  << " input messages\n";
    }
}
std::optional<GameMessage>
GameSession::convertMessageToGameMessage(const ClientMessage& clientMsg) const {
    std::string playerID = std::to_string(clientMsg.clientID);

    if (auto* val = std::get_if<ResponseTextInputMessage>(&clientMsg.message.data)) {
        return GameMessage{TextInputMessage{
            String{playerID},
            String{val->promptReference},
            String{val->input}}};
    }
    else if (auto* val = std::get_if<ResponseChoiceInputMessage>(&clientMsg.message.data)) {
        return GameMessage{ChoiceInputMessage{
            String{playerID},
            String{val->promptRef},
            String{val->choice}}};
    }
    else if (auto* val = std::get_if<ResponseRangeInputMessage>(&clientMsg.message.data)) {
        return GameMessage{RangeInputMessage{
            String{playerID},
            String{val->promptRef},
            Integer{val->value}}};
    }

    return std::nullopt;
}

Message
GameSession::convertGameMessageToMessage(const GameMessage& engineMsg) const {
    Message msg;
    msg.type = MessageType::Empty;

    if (auto* req = std::get_if<GetTextInputMessage>(&engineMsg.inner)) {
        msg.type = MessageType::RequestTextInput;
        msg.data = RequestTextInputMessage{req->prompt.value};
    }

    else if (auto* req = std::get_if<GetChoiceInputMessage>(&engineMsg.inner)) {
        msg.type = MessageType::RequestChoiceInput;
        msg.data = RequestChoiceInputMessage{req->prompt.value};
    }

    else if (auto* req = std::get_if<GetRangeInputMessage>(&engineMsg.inner)) {
        msg.type = MessageType::RequestRangeInput;
        msg.data = RequestRangeInputMessage{
            req->prompt.value,
            req->minValue.value,
            req->maxValue.value};
    }

    return msg;
}

std::vector<ClientMessage>
GameSession::collectOutgoingMessages() {
    std::vector<ClientMessage> outgoing;

    if (!m_interpreter.needsIO()) {
        return outgoing;
    }

    auto requests = m_inputManager.getPendingRequests();

    for (const auto& req : requests) {
        Message netMsg = convertGameMessageToMessage(req);

        std::string targetPlayerID;
        if (auto* t = std::get_if<GetTextInputMessage>(&req.inner)) {
            targetPlayerID = t->playerID.value;
        }
        else if (auto* c = std::get_if<GetChoiceInputMessage>(&req.inner)) {
            targetPlayerID = c->playerID.value;
        }
        else if (auto* r = std::get_if<GetRangeInputMessage>(&req.inner)) {
            targetPlayerID = r->playerID.value;
        }
        else if (auto* v = std::get_if<GetVoteInputMessage>(&req.inner)) {
            targetPlayerID = v->playerID.value;
        }

        uintptr_t targetClientID = getClientID(targetPlayerID, m_players);

        if (targetClientID != 0) {
            outgoing.push_back(ClientMessage{targetClientID, netMsg});
        } else {
            std::cerr << "[GameSession] Warning: Could not find client for player ID: " << targetPlayerID << "\n";
        }
    }

    m_inputManager.clearPendingRequests();

    return outgoing;
}

