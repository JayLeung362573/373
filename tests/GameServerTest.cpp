#include <gtest/gtest.h>
#include "GameServer.h"

TEST(GameServerTest, StartGameWithoutLobbyFails){
    GameServer server;
    uintptr_t clientID = 888;

    std::vector<ClientMessage> incoming = {
            {clientID, {MessageType::StartGame, StartGameMessage{"player 1"}}}
    };

    std::vector<ClientMessage> outgoingMsg = server.tick(incoming);

    ASSERT_GT(outgoingMsg.size(), 0);
    ASSERT_EQ(outgoingMsg[0].clientID, clientID);
    ASSERT_EQ(outgoingMsg[0].message.type, MessageType::Error);

    ASSERT_TRUE(std::holds_alternative<ErrorMessage>(outgoingMsg[0].message.data));
    auto& errorPayload = std::get<ErrorMessage>(outgoingMsg[0].message.data);

    ASSERT_EQ(errorPayload.reason, "You must be in a lobby to start a game");
}

TEST(GameServerTest, MultipleClientsStartGame){
    GameServer server;

    std::vector<ClientMessage> incoming = {
            {1, {MessageType::StartGame, StartGameMessage{"player 1"}}},
            {2, {MessageType::StartGame, StartGameMessage{"player 2"}}},
            {3, {MessageType::StartGame, StartGameMessage{"player 3"}}},

    };

    std::vector<ClientMessage> outgoingMsg = server.tick(incoming);

    ASSERT_EQ(outgoingMsg.size(), 3);
    bool foundClient1 = false, foundClient2 = false, foundClient3 = false;
    for(const auto& msg : outgoingMsg){
        if(msg.clientID == 1) foundClient1 = true;
        if(msg.clientID == 2) foundClient2 = true;
        if(msg.clientID == 3) foundClient3 = true;
    }
    ASSERT_TRUE(foundClient1);
    ASSERT_TRUE(foundClient2);
    ASSERT_TRUE(foundClient3);
}


// Helper to add a lobby + host + player
static void setupLobby(GameServer &server, uintptr_t hostID) {
    server.tick({{hostID, {MessageType::JoinLobby, JoinLobbyMessage{"Host", "lobby_test", 0}}}});
}

// Unit testings for starting the server
TEST(GameServerStartTest, ErrorIfClientNotInLobby) {
    GameServer server;
    auto out = server.handleStartGameMessages(123, StartGameMessage{"Host"});

    ASSERT_EQ(out.size(), 1);
    ASSERT_EQ(out[0].message.type, MessageType::Error);
}

TEST(GameServerStartTest, ErrorIfNotHostToStartGame) {
    GameServer server;
    // Host joins lobby
    server.tick({{1, {MessageType::JoinLobby, JoinLobbyMessage{"Host", "lobby_test"}}}});
    // Player 2 joins same lobby
    server.tick({{2, {MessageType::JoinLobby, JoinLobbyMessage{"Player2", "lobby_test"}}}});

    // Non-host client tries to start
    auto out = server.handleStartGameMessages(2, StartGameMessage{"Player2"});

    ASSERT_EQ(out.size(), 1);
    ASSERT_EQ(out[0].message.type, MessageType::Error);
}

TEST(GameServerStartTest, ErrorIfGameAlreadyStarted) {
    GameServer server;
    setupLobby(server, 1);

    // Start game once
    server.handleStartGameMessages(1, StartGameMessage{"Host"});

    // Shouldn't be able to start again
    auto out = server.handleStartGameMessages(1, StartGameMessage{"Host"});

    ASSERT_EQ(out[0].message.type, MessageType::Error);
}

TEST(GameServerStartTest, SuccessfulStartSendsNotifications) {
    GameServer server;
    setupLobby(server, 1);
    server.tick({{2, {MessageType::JoinLobby, JoinLobbyMessage{"Player2", "lobby_test", 0}}}});
    auto out = server.handleStartGameMessages(1, StartGameMessage{"Host"});

    ASSERT_GE(out.size(), 1); // start notifications + initial game messages

    bool foundStart = false;
    for (auto &msg : out) {
        if (msg.message.type == MessageType::StartGame) {
            foundStart = true;
        }
    }
    ASSERT_TRUE(foundStart);
}

TEST(GameServerStartTest, MultipleActiveSessionsTracked) {
    GameServer server;
    server.handleJoinLobbyMessages(1, {"host_1", "Lobby1"});
    server.handleJoinLobbyMessages(2, {"host_2", "Lobby2"});

    auto r1 = server.handleStartGameMessages(1, {"host_1"});
    auto r2 = server.handleStartGameMessages(2, {"host_2"});

    EXPECT_FALSE(r1.empty());
    EXPECT_FALSE(r2.empty());
}

// Integration test: Lobby → StartGame → GameSession → Requests → Player input → GameOver
TEST(GameServerTestIntegrationTest, FullGameFlowTwoPlayers) {
    GameServer server;

    // Join lobby
    server.tick({{1, {MessageType::JoinLobby, JoinLobbyMessage{"P1", "L", 0}}}});
    server.tick({{2, {MessageType::JoinLobby, JoinLobbyMessage{"P2", "L", 0}}}});

    // Host starts game
    auto startOut = server.handleStartGameMessages( 1, StartGameMessage{"P1"});

    ASSERT_FALSE(startOut.empty());

    // Should see StartGame broadcast and input requests
    bool foundRangeRequest = false;

    for (auto &msg : startOut) {
        if (msg.message.type == MessageType::RequestRangeInput) {
            foundRangeRequest = true;
        }
    }
    ASSERT_TRUE(foundRangeRequest);

    // 3. Both players respond to input
    std::vector<ClientMessage> responses = {
        {1, {MessageType::ResponseRangeInput, ResponseRangeInputMessage{42, "Player1: Enter your number"}}},
        {2, {MessageType::ResponseRangeInput, ResponseRangeInputMessage{55, "Player2: Enter your number"}}},
    };

    auto out2 = server.tick(responses);

    // Should produce GameOver eventually
    bool foundOver = false;
    for (auto &msg : out2) {
        if (msg.message.type == MessageType::GameOver) {
            foundOver = true;
        }
    }

    ASSERT_TRUE(foundOver);
}
