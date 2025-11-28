#include "GameSession.h"
#include <iostream>
#include <utility>

GameSession::GameSession(LobbyID lobbyID, ast::GameRules rules, std::vector<LobbyMember> players)
    : m_lobbyID(std::move(lobbyID))
    , m_players(std::move(players))
    , m_interpreter(m_inputManager, convertRulesToProgram(rules))
    {
        /// map player with their id
        for(const auto& player : m_players){
            m_playerIDs.insert(player.clientID);
        }

        for (size_t i = 0; i < m_players.size(); ++i) {
            Map<String, Value> playerMap;

            playerMap.setAttribute(String{"id"}, Value{String{std::to_string(m_players[i].clientID)}});
            playerMap.setAttribute(String{"name"}, Value{String{m_players[i].name}});

            std::string varName = "player" + std::to_string(i + 1);
            m_interpreter.setVariable(String{varName}, Value{playerMap});

            std::cout << "[GameSession] Registered " << varName << " -> Client " << m_players[i].clientID << "\n";
        }

    std::cout << "[GameSession] Created session for lobby " << m_lobbyID
              << " with " << m_players.size() << " players\n";}

std::optional<uintptr_t>
getClientID(const std::string& playerID, const std::vector<LobbyMember>& players) {
    for (const auto& p : players) {
        if (p.name == playerID) return p.clientID;

        if (std::to_string(p.clientID) == playerID) {
            return p.clientID;
        }
    }
    return std::nullopt;
}

std::vector<ClientMessage>
GameSession::start() {
    std::cout << "[GameSession] Starting game execution\n";

    m_inputManager.sendOutput(String{"Game starts!"});

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
    return m_interpreter.isDone();
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
        /// if client(player) isn't in this session, skip
        if (!m_playerIDs.contains(clientMsg.clientID)) {
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

    auto outputs = m_inputManager.popPendingOutputs();
    for(const auto& text : outputs){
        Message gameOutputMsg;
        gameOutputMsg.type = MessageType::GameOutput;
        gameOutputMsg.data = GameOutputMessage{text};

        for(const auto& player : m_players){
            outgoing.push_back(ClientMessage{player.clientID, gameOutputMsg});
        }
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

        auto targetClientID = getClientID(targetPlayerID, m_players);

        if (targetClientID.has_value()) {
            outgoing.push_back(ClientMessage{*targetClientID, netMsg});
        } else {
            std::cerr << "[GameSession] Warning: Could not find client for player ID: " << targetPlayerID << "\n";
        }
    }

    m_inputManager.clearPendingRequests();

    if(isFinished()){
        Message gameOverMsg;
        gameOverMsg.type = MessageType::GameOver;
        gameOverMsg.data = GameOverMessage{"Game Over"};

        for(const auto& player : m_players){
            outgoing.push_back(ClientMessage{player.clientID, gameOverMsg});
        }
    }

    return outgoing;
}

