#include <iostream>
#include "GameServer.h"
#include "Message.h"
#include <unordered_map>

namespace{
    std::string formatLobbyList(const std::vector<LobbyInfo>& lobbies) {
        if (lobbies.empty()) {
            return "No existing lobbies";
        }

        std::string output = "Current active lobbies:\n";
        for (const auto& lobby : lobbies) {
            output += " - " + lobby.lobbyName + " (ID: " + lobby.lobbyID + ") | " +
                      "Players: " + std::to_string(lobby.currentPlayers) + "/" +
                      std::to_string(lobby.maxPlayers) + "\n";
        }
        return output;
    }
}

GameServer::GameServer() = default;

struct MessageHandlerVisitor {
    uintptr_t clientID;
    GameServer* server;

    std::vector<ClientMessage> responses;

    void operator()(const CreateLobbyMessage& data) {
        responses = server->handleCreateLobbyMessages(clientID);
    }

    void operator()(const StartJoinLobbyMessage&) {
        responses = server->handleStartJoinLobbyMessages(clientID);
    }

    void operator()(const JoinLobbyMessage& data) {
        responses = server->handleJoinLobbyMessages(clientID, data);
    }

    void operator()(const LeaveLobbyMessage& data) {
        responses = server->handleLeaveLobbyMessages(clientID, data);
    }

    void operator()(const GetLobbyStateMessage& data) {
        responses = server->handleGetLobbyStateMessages(clientID, data);
    }

    void operator()(const BrowseLobbiesMessage& data) {
        responses = server->handleBrowseLobbiesMessages(clientID, data);
    }

    void operator()(const StartGameMessage& data) {
        responses = server->handleStartGameMessages(clientID, data);
    }

    void operator()(const std::monostate& data) {}

    void operator()(const LobbyStateMessage& data) {}

    void operator()(const ErrorMessage& data) {}

    void operator()(const UpdateCycleMessage& data) {
        std::cout << "[GameServer]: Client should not send UpdateCycle\n";
    }

    // The GameServer ignores these here because they are passed to GameSession::tick later
    // Responses from Client (Inputs)
    void operator()(const ResponseTextInputMessage&) {}
    void operator()(const ResponseChoiceInputMessage&) {}
    void operator()(const ResponseRangeInputMessage&) {}

    // Requests from Server
    void operator()(const RequestTextInputMessage&) {}
    void operator()(const RequestChoiceInputMessage&) {}
    void operator()(const RequestRangeInputMessage&) {}

    // Game output and game over message
    void operator()(const GameOutputMessage&) {}
    void operator()(const GameOverMessage&) {}
};

std::vector<ClientMessage>
GameServer::handleClientMessages(const std::vector<ClientMessage> &incomingMessages) {
    std::vector<ClientMessage> outgoingMessages;

    for(const auto& clientMsg : incomingMessages){
        /// check if this user is joining lobby
        if (m_pendingJoins.count(clientMsg.clientID)) {
            auto responses = handleJoinInput(clientMsg.clientID, clientMsg.message);
            outgoingMessages.insert(outgoingMessages.end(), responses.begin(), responses.end());
            continue;
        }

        /// check if this user is in the lobby creation
        if (m_pendingCreations.count(clientMsg.clientID)) {
            auto creationResponses = handleCreationInput(clientMsg.clientID, clientMsg.message);
            outgoingMessages.insert(outgoingMessages.end(), creationResponses.begin(), creationResponses.end());
            continue;
        }

        MessageHandlerVisitor visitor {clientMsg.clientID, this};

        /// visit the clientMsg.message.data and call the correct handler
        std::visit(visitor, clientMsg.message.data);

        outgoingMessages.insert(outgoingMessages.end(),
                                std::make_move_iterator(visitor.responses.begin()),
                                std::make_move_iterator(visitor.responses.end()));
    }
    return outgoingMessages;
}

std::vector<ClientMessage>
GameServer::handleCreateLobbyMessages(uintptr_t clientID) {
    /// check if in a lobby
    if (m_lobbyRegistry.findLobbyForClient(clientID)) {
        Message err;
        err.type = MessageType::Error;
        err.data = ErrorMessage{"You are already in a lobby. Leave first."};
        return { ClientMessage{clientID, err} };
    }

    m_pendingCreations.erase(clientID);
    LobbyCreationState state;
    state.currentStep = LobbyCreationState::Step::AskingForName;
    m_pendingCreations[clientID] = state;

    std::cout << "[GameServer] Client " << clientID << " started lobby creation.\n";

    Message req;
    req.type = MessageType::RequestTextInput;
    req.data = RequestTextInputMessage{"Enter Lobby Name"};

    return { ClientMessage{clientID, req} };
}

std::vector<ClientMessage>
GameServer::handleCreationInput(uintptr_t clientID, const Message& creationInput){
    auto& state = m_pendingCreations[clientID];

    if(state.currentStep == LobbyCreationState::Step::AskingForName){
        if(auto* text = std::get_if<ResponseTextInputMessage>(&creationInput.data)){
            if(text->input.empty()){ /// empty input for lobbyName
                Message err;
                err.type = MessageType::RequestTextInput;
                err.data = RequestTextInputMessage{"Name cannot be empty. Enter Lobby Name"};
                return { ClientMessage{clientID, err} };
            }

            /// save valid lobbyName and next step
            state.lobbyName = text->input;
            state.currentStep = LobbyCreationState::Step::AskingForType;
            std::cout << "[GameServer] Client " << clientID << " set lobby name: " << state.lobbyName << "\n";

            Message request;
            request.type = MessageType::RequestChoiceInput;
            request.data = RequestChoiceInputMessage{ /// for current demo games
                    "Select Game Type",
                    {"1 (Number Battle)", "2 (Choice Battle)"}
            };
            return {ClientMessage{clientID, request}};
        }
    }
    else if(state.currentStep == LobbyCreationState::Step::AskingForType){
        if (auto* choice = std::get_if<ResponseChoiceInputMessage>(&creationInput.data)){
            int typeInt = 1;
            if (choice->choice.starts_with("2")){
                typeInt = 2;
            }

            std::cout << "[GameServer] Client " << clientID << " selected type: " << typeInt << "\n";

            Lobby* lobby = m_lobbyRegistry.createLobby(
                    clientID,
                    static_cast<GameType>(typeInt),
                    state.lobbyName
            );


            Message lobbyStateMsg;
            lobbyStateMsg.type = MessageType::LobbyState;
            lobbyStateMsg.data = LobbyStateMessage{
                    {lobby->getInfo()},
                    lobby->getInfo().lobbyID
            };

            Message textMsg;
            textMsg.type = MessageType::GameOutput;
            textMsg.data = GameOutputMessage{"Lobby '" + state.lobbyName + "' Created Successfully!"};

            m_pendingCreations.erase(clientID);

            return {ClientMessage{clientID, lobbyStateMsg}, ClientMessage{clientID, textMsg}};
        }
    }
    return {};
}

std::vector<ClientMessage>
GameServer::handleStartJoinLobbyMessages(uintptr_t clientID){
    m_pendingJoins.erase(clientID);
    m_pendingCreations.erase(clientID);

    LobbyJoinState state;
    state.currentStep = LobbyJoinState::Step::AskingForPlayerName;
    m_pendingJoins[clientID] = state;

    std::cout << "[GameServer] Client " << clientID << " started join process.\n";

    Message playerNameRequest;
    playerNameRequest.type = MessageType::RequestTextInput;
    playerNameRequest.data = RequestTextInputMessage{"Enter your name"};

    return {ClientMessage{clientID, playerNameRequest}};
}

std::vector<ClientMessage>
GameServer::handleJoinInput(uintptr_t clientID, const Message& joinInput){
    auto& state = m_pendingJoins[clientID];

    /// 1. enter and validate player name
    if(state.currentStep == LobbyJoinState::Step::AskingForPlayerName){
        if(auto* text = std::get_if<ResponseTextInputMessage>(&joinInput.data)){
            if(text->input.empty()){
                Message err;
                err.type = MessageType::RequestTextInput;
                err.data = RequestTextInputMessage{"Name empty. Enter Your Name"};
                return {ClientMessage{clientID, err}};
            }

            state.playerName = text->input;
            state.currentStep = LobbyJoinState::Step::AskingForLobbyName;
            std::cout << "[GameServer] Client " << clientID << " set join player name as: " << state.playerName << "\n";

            Message lobbyNameRequest;
            lobbyNameRequest.type = MessageType::RequestTextInput;
            lobbyNameRequest.data = RequestTextInputMessage{"Enter lobby name: "};
            return {ClientMessage{clientID, lobbyNameRequest}};
        }
    }
    else if(state.currentStep == LobbyJoinState::Step::AskingForLobbyName){
        if(auto* text = std::get_if<ResponseTextInputMessage>(&joinInput.data)){
            std::string targetLobby = text->input;
            std::cout << "[GameServer] Client " << clientID << " attempting to join: " << targetLobby << "\n";

            Lobby* lobby = m_lobbyRegistry.joinLobby(clientID, targetLobby);

            if(!lobby){
                auto lobbies = m_lobbyRegistry.browseLobbies(std::nullopt);
                for(const auto& info : lobbies){
                    if(info.lobbyName == targetLobby){
                        lobby = m_lobbyRegistry.joinLobby(clientID, info.lobbyID);
                        break;
                    }
                }
            }

            if(lobby){
                Message success;
                success.type = MessageType::LobbyState;
                success.data = LobbyStateMessage{
                        {lobby->getInfo()},
                        lobby->getInfo().lobbyID
                };

                Message msg;
                msg.type = MessageType::GameOutput;
                msg.data = GameOutputMessage{"Joined Lobby '" + lobby->getInfo().lobbyName + "'!"};

                m_pendingJoins.erase(clientID); /// Cleanup
                return {ClientMessage{clientID, success}, ClientMessage{clientID, msg}};
            } else{
                Message err;
                err.type = MessageType::RequestTextInput;
                err.data = RequestTextInputMessage{"Lobby not found. Enter Lobby Name (or ID) to Join"};
                return { ClientMessage{clientID, err} };
            }
        }
    }
    return {};
}

std::vector<ClientMessage>
GameServer::handleJoinLobbyMessages(uintptr_t clientID, const JoinLobbyMessage& joinLobbyMsg) {
    std::cout << "[GameServer] Player {" << joinLobbyMsg.playerName
              << "} attempting to join lobby (clientID = " << clientID << ")\n";

    LobbyID lobbyID;
    Lobby* lobby = nullptr;
    std::vector<ClientMessage> outgoingMessages;

    if(joinLobbyMsg.lobbyName.empty()){
        /// Empty name: create new
        lobby = m_lobbyRegistry.createLobby(
                clientID,
                static_cast<GameType>(joinLobbyMsg.gameType),
                joinLobbyMsg.playerName + "'s Lobby"
        );
    } else {
        /// try joining with lobbyID
        lobbyID = joinLobbyMsg.lobbyName;
        lobby = m_lobbyRegistry.joinLobby(clientID, lobbyID);

        /// try joining with lobbyName
        if(!lobby){
            auto lobbies = m_lobbyRegistry.browseLobbies(static_cast<GameType>(joinLobbyMsg.gameType));

            for (const auto& info : lobbies) {
                if (info.lobbyName == joinLobbyMsg.lobbyName) {
                    std::cout << "[GameServer] Found lobby by name: " << info.lobbyName << " -> ID: " << info.lobbyID << "\n";
                    lobby = m_lobbyRegistry.joinLobby(clientID, info.lobbyID);
                    break;
                }
            }
        }

        /// still not found by id and name
        if(!lobby){
            std::cout << "[GameServer] Lobby '" << lobbyID << "' not found. Creating new lobby.\n";
            lobby = m_lobbyRegistry.createLobby(
                    clientID,
                    static_cast<GameType>(joinLobbyMsg.gameType),
                    joinLobbyMsg.lobbyName
            );
        }
    }

    if(!lobby){
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"[handleJoinLobby: Lobby not found or is full"};
        outgoingMessages.push_back(ClientMessage{clientID, errorMsg});
        return outgoingMessages;
    }

    lobbyID = lobby->getInfo().lobbyID;

    Message lobbyStatePayload;
    lobbyStatePayload.type = MessageType::LobbyState;
    lobbyStatePayload.data = LobbyStateMessage{
            {lobby->getInfo()},
            lobbyID
    };

    auto players = lobby->getAllPlayer();
    std::cout << "[GameServer] Broadcasting new lobby state to "
              << players.size() << " clients.\n";

    for(const auto& player : players) {
        outgoingMessages.push_back(ClientMessage{player.clientID, lobbyStatePayload});
    }

    return outgoingMessages;
}

std::vector<ClientMessage>
GameServer::handleBrowseLobbiesMessages(uintptr_t clientID, const BrowseLobbiesMessage &browseLobbiesMsg) const {
    std::cout << "[GameServer] Client " << clientID
              << " browsing lobbies\n";

    auto lobbies = m_lobbyRegistry.browseLobbies(std::nullopt);

    std::string text = formatLobbyList(lobbies);

    Message response;
    response.type = MessageType::GameOutput;
    response.data = GameOutputMessage{text};
    return {ClientMessage{clientID, response}};
}

std::vector<ClientMessage>
GameServer::showCurrentLobbies(uintptr_t clientID){
    auto lobbies = m_lobbyRegistry.browseLobbies(std::nullopt);

    std::string text = formatLobbyList(lobbies);

    Message response;
    response.type = MessageType::GameOutput;
    response.data = GameOutputMessage{text};

    return {ClientMessage{clientID, response}};
}

std::vector<ClientMessage>
GameServer::handleStartGameMessages(uintptr_t clientID, const StartGameMessage& startGameMsg){
    std::cout << "[GameServer] Player " << startGameMsg.playerName << " (clientID: " << clientID << ")"
              << " requesting to start game\n";

    /// 1. check if this client is in a lobby
    auto lobbyID = m_lobbyRegistry.findLobbyForClient(clientID);
    if(!lobbyID){
        std::cout << "[GameServer] Error: Client not in any lobby\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"You must be in a lobby to start a game"};
        return {ClientMessage{clientID, errorMsg}};
    }

    /// 2. check this lobby
    auto lobby = m_lobbyRegistry.getLobby(*lobbyID);
    if(!lobby){
        std::cout << "[GameServer] Error: Lobby: " << *lobbyID << "not found\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"Lobby not found"};
        return {ClientMessage{clientID, errorMsg}};
    }

    /// 3. check if it is a host to start the game
    if(lobby->getHostID() != clientID){
        std::cout << "[GameServer] Error: Only host can start game\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"Only the lobby host can start the game"};
        return {ClientMessage{clientID, errorMsg}};
    }

    /// 4. check if the game already started
    if(m_activeSessions.find(*lobbyID) != m_activeSessions.end()){
        std::cout << "[GameServer] Error: Game already started for lobby "
                  << *lobbyID << "\n";

        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"Game already started"};
        return {ClientMessage{clientID, errorMsg}};
    }

    /// 5. get all players (host and players, excluding audience)
    auto players = lobby->getAllPlayer();

    if (players.size() < 2) {
        std::cout << "[GameServer] Error: Not enough players to start (Requires 2)\n";
        Message errorMsg;
        errorMsg.type = MessageType::Error;
        errorMsg.data = ErrorMessage{"Not enough players to start. You need at least 2."};
        return {ClientMessage{clientID, errorMsg}};
    }

    std::cout << "[GameServer] Starting game for lobby " << *lobbyID
              << " with " << players.size() << " players\n";

    /// 6. create game rules
    ast::GameRules rules = createGameRules(lobby->getInfo().gameType);

    /// 7. create and start session
    auto session = std::make_unique<GameSession>(*lobbyID, std::move(rules), players);
    std::vector<ClientMessage> initialGameMessages = session->start();

    /// 8. map and track this session
    m_activeSessions[*lobbyID] = std::move(session);

    /// 9. create 'startMsg' for each player and store them in 'responses',
    /// the networking will receive this 'responses' and send(notify) to each player
    /// and initial game prompt for inputs
    std::vector<ClientMessage> responses;

    for(const auto& player : players){
        std::cout << "[GameServer] Notifying player " << player.clientID << "\n";

        Message startMsg;
        startMsg.type = MessageType::StartGame;
        startMsg.data = StartGameMessage{"Game started"};

        responses.push_back(ClientMessage{player.clientID, startMsg});
    }

    responses.insert(responses.end(),
                     initialGameMessages.begin(),
                     initialGameMessages.end());

    return responses;
}

std::vector<ClientMessage>
GameServer::tick(const std::vector<ClientMessage> &incomingMessages) {
    std::vector<ClientMessage> outgoing = handleClientMessages(incomingMessages);

    /// group game messages by LobbyID
    std::unordered_map<LobbyID, std::vector<ClientMessage>> sessionInputs;

    for(const auto& msg : incomingMessages){
        /// ignore non-game message, such as Join/LeaveLobby
        if(!isGameInputMessage(msg.message)){
            continue;
        }

        /// find target lobby for this message
        auto lobbyID = m_lobbyRegistry.findLobbyForClient(msg.clientID);
        if(lobbyID){
            sessionInputs[*lobbyID].push_back(msg);
        }
    }

    /// tick session with the specific messages in this specific lobby
    auto it = m_activeSessions.begin();
    while(it != m_activeSessions.end()){
        if(it->second->isFinished()){
            std::cout << "[GameServer] Session " << it->first << " finished and get cleaned\n";
            it = m_activeSessions.erase(it);
        } else{
            /// check if it has input waiting for this specific lobby
            /// if not, initialize it with an empty vector
            static std::vector<ClientMessage> empty;

            std::vector<ClientMessage>& specificMessages =
                    sessionInputs.count(it->first) ? sessionInputs[it->first]: empty;

            if(sessionInputs.count(it->first)){
                specificMessages = sessionInputs[it->first];
            }

            auto sessionUpdates = it->second->tick(specificMessages);

            outgoing.insert(outgoing.end(),
                            sessionUpdates.begin(),
                            sessionUpdates.end());
            ++it;
        }
    }
    return outgoing;
}

bool
GameServer::isGameInputMessage(const Message &msg) const {
    switch(msg.type){
        case MessageType::ResponseChoiceInput:
        {
            return true;
        }
        case MessageType::ResponseRangeInput:
        {
            return true;
        }
        case MessageType::ResponseTextInput:
        {
            return true;
        }
        default:
            return false;
    }
}

/// make choice input game and text input game to test
/// when .game files are ready to use, change these
ast::GameRules
GameServer::createNumberBattleRules(){
    std::vector<std::unique_ptr<ast::Statement>> statements;

    auto player1Var = ast::makeVariable(Name{"player1"});
    auto player2Var = ast::makeVariable(Name{"player2"});

    /// ask player 1 and 2 for number input
    statements.push_back(ast::makeInputRange(
            ast::cloneVariable(player1Var.get()),
            ast::makeVariable(Name{"p1_val"}),
            String{"Player1: Enter your number"},
            ast::makeConstant(Value{Integer{0}}),
            ast::makeConstant(Value{Integer{100}})
    ));
    statements.push_back(ast::makeInputRange(
            ast::cloneVariable(player2Var.get()),
            ast::makeVariable(Name{"p2_val"}),
            String{"Player2: Enter your number"},
            ast::makeConstant(Value{Integer{0}}),
            ast::makeConstant(Value{Integer{100}})
    ));

    std::vector<ast::Match::Candidate> candidates;

    auto P1Wins = ast::makeComparison(
            ast::makeVariable(Name{"p2_val"}),
            ast::makeVariable(Name{"p1_val"}),
            ast::Comparison::Kind::LT
    );
    std::vector<std::unique_ptr<ast::Statement>> statementP1Wins;
    statementP1Wins.push_back(ast::makeAssignment(
            ast::makeVariable(Name{"Game Result"}),
            ast::makeConstant(Value{String{"P1 won"}})
    ));

    candidates.push_back({std::move(P1Wins), std::move(statementP1Wins)});

    auto P2Wins = ast::makeComparison(
            ast::makeVariable(Name{"p1_val"}),
            ast::makeVariable(Name{"p2_val"}),
            ast::Comparison::Kind::LT
    );
    std::vector<std::unique_ptr<ast::Statement>> statementP2Wins;
    statementP2Wins.push_back(ast::makeAssignment(
            ast::makeVariable(Name{"Game Result"}),
            ast::makeConstant(Value{String{"P2 won"}})
    ));

    candidates.push_back({std::move(P2Wins), std::move(statementP2Wins)});

    auto Tie = ast::makeComparison(
            ast::makeVariable(Name{"p1_val"}),
            ast::makeVariable(Name{"p2_val"}),
            ast::Comparison::Kind::EQ
    );
    std::vector<std::unique_ptr<ast::Statement>> statementTie;
    statementTie.push_back(ast::makeAssignment(
            ast::makeVariable(Name{"Game result"}),
            ast::makeConstant(Value{String{"Tie"}})
    ));

    candidates.push_back({std::move(Tie), std::move(statementTie)});

    statements.push_back(ast::makeMatch(
            ast::makeConstant(Value{Boolean{true}}),
            std::move(candidates)
    ));

    std::cout << "[GameServer] Created simple game with "
              << statements.size() << " statements\n";

    return ast::GameRules{std::move(statements)};
}

ast::GameRules
GameServer::createChoiceBattleRules(){
    std::vector<std::unique_ptr<ast::Statement>> statements;

    auto player1Var = ast::makeVariable(Name{"player1"});
    auto player2Var = ast::makeVariable(Name{"player2"});

    /// game 2: ask player to choose the number, and compare, larger one wins
    auto choicesList = ast::makeConstant(Value{List<Value>{
            Value{String{"1"}},
            Value{String{"2"}},
            Value{String{"3"}}
    }});

    statements.push_back(ast::makeInputChoice(
            ast::cloneVariable(player1Var.get()),
            ast::makeVariable(Name{"p1_val"}),
            String{"Player 1: Choose 1, 2, or 3"},
            ast::cloneExpression(choicesList.get())
    ));

    statements.push_back(ast::makeInputChoice(
            ast::cloneVariable(player2Var.get()),
            ast::makeVariable(Name{"p2_val"}),
            String{"Player 2: Choose 1, 2, or 3"},
            ast::cloneExpression(choicesList.get())
    ));

    std::vector<ast::Match::Candidate> candidates;

    auto P1Wins = ast::makeComparison(
            ast::makeVariable(Name{"p2_val"}),
            ast::makeVariable(Name{"p1_val"}),
            ast::Comparison::Kind::LT
    );
    std::vector<std::unique_ptr<ast::Statement>> statementP1Wins;
    statementP1Wins.push_back(ast::makeAssignment(
            ast::makeVariable(Name{"Game result"}),
            ast::makeConstant(Value{String{"P1 wins"}})
    ));

    candidates.push_back({std::move(P1Wins), std::move(statementP1Wins)});

    auto P2Wins = ast::makeComparison(
            ast::makeVariable(Name{"p1_val"}),
            ast::makeVariable(Name{"p2_val"}),
            ast::Comparison::Kind::LT
    );
    std::vector<std::unique_ptr<ast::Statement>> statementP2Wins;
    statementP2Wins.push_back(ast::makeAssignment(
            ast::makeVariable(Name{"Game result"}),
            ast::makeConstant(Value{String{"P2 wins"}})
    ));

    candidates.push_back({std::move(P2Wins), std::move(statementP2Wins)});

    auto Tie = ast::makeComparison(
            ast::makeVariable(Name{"p1_val"}),
            ast::makeVariable(Name{"p2_val"}),
            ast::Comparison::Kind::EQ
    );
    std::vector<std::unique_ptr<ast::Statement>> statementTie;
    statementTie.push_back(ast::makeAssignment(
            ast::makeVariable(Name{"Game result"}),
            ast::makeConstant(Value{String{"Tie"}})
    ));

    candidates.push_back({std::move(Tie), std::move(statementTie)});

    statements.push_back(ast::makeMatch(
            ast::makeConstant(Value{Boolean{true}}),
            std::move(candidates)
    ));

    std::cout << "[GameServer] Created simple game with "
              << statements.size() << " statements\n";

    return ast::GameRules{std::move(statements)};
}

ast::GameRules
GameServer::createGameRules(GameType type) {
    std::cout << "[GameServer] Creating rules for GameType: " << (int)type << "\n";
    switch(type) {
        case GameType::Default:
            return createNumberBattleRules();

        case GameType::NumberBattle:
            return createNumberBattleRules();

        case GameType::ChoiceBattle:
            return createChoiceBattleRules();

        default:
            std::cout << "[GameServer] Unknown game type, creating default game\n";
            return createNumberBattleRules();
    }
}

