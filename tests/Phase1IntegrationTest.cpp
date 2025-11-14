#include <gtest/gtest.h>
#include "GameServer.h"
#include "GameSession.h"
#include "Message.h"
#include "GameEngine/GameRuntime.h"
#include "GameEngine/Rules.h"

/**
 * Phase 1 Integration Tests
 * Tests the complete flow: Lobby → GameSession → GameRuntime → GameInterpreter
 */

// Test fixture for Phase 1 tests
class Phase1IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        server = std::make_unique<GameServer>();
    }

    // Helper: Create a lobby with host
    std::string createLobby(uintptr_t hostID, const std::string& hostName) {
        Message createMsg;
        createMsg.type = MessageType::JoinLobby;
        createMsg.data = JoinLobbyMessage{hostName, ""};  // Empty name = create new

        auto responses = server->tick({ClientMessage{hostID, createMsg}});

        // Extract lobby ID from response
        if (!responses.empty() &&
            std::holds_alternative<LobbyStateMessage>(responses[0].message.data)) {
            return std::get<LobbyStateMessage>(responses[0].message.data).currentLobbyID;
        }
        return "";
    }

    // Helper: Join existing lobby
    void joinLobby(uintptr_t clientID, const std::string& playerName, const std::string& lobbyID) {
        Message joinMsg;
        joinMsg.type = MessageType::JoinLobby;
        joinMsg.data = JoinLobbyMessage{playerName, lobbyID};

        server->tick({ClientMessage{clientID, joinMsg}});
    }

    // Helper: Start game
    std::vector<ClientMessage> startGame(uintptr_t hostID, const std::string& hostName) {
        Message startMsg;
        startMsg.type = MessageType::JoinGame;
        startMsg.data = JoinGameMessage{hostName};

        return server->tick({ClientMessage{hostID, startMsg}});
    }

    std::unique_ptr<GameServer> server;

    // Test client IDs
    const uintptr_t ALICE_ID = 100;
    const uintptr_t BOB_ID = 200;
    const uintptr_t CAROL_ID = 300;
};

// ============================================================================
// BASIC GAME SESSION TESTS
// ============================================================================

TEST_F(Phase1IntegrationTest, GameSessionCreationAndExecution)
{
    // Create simple game rules
    std::vector<std::unique_ptr<ast::Statement>> statements;
    statements.push_back(
        ast::makeAssignment(
            ast::makeVariable(Name{"winner"}),
            ast::makeConstant(Value{String{"Player1"}})
        )
    );

    GameRules rules{std::span(statements)};

    // Create game session
    std::vector<LobbyMember> players = {
        LobbyMember{ALICE_ID, LobbyRole::Host, true}
    };

    GameSession session("test_lobby", rules, players);

    EXPECT_FALSE(session.isFinished());
    EXPECT_EQ(session.getLobbyID(), "test_lobby");

    // Start game
    session.start();

    EXPECT_TRUE(session.isFinished());
}

TEST_F(Phase1IntegrationTest, GameRuntimeExecutesStatements)
{
    // Create game with 2 statements
    std::vector<std::unique_ptr<ast::Statement>> statements;
    statements.push_back(
        ast::makeAssignment(
            ast::makeVariable(Name{"score"}),
            ast::makeConstant(Value{Integer{100}})
        )
    );
    statements.push_back(
        ast::makeAssignment(
            ast::makeVariable(Name{"winner"}),
            ast::makeConstant(Value{String{"Alice"}})
        )
    );

    GameRules rules{std::span(statements)};
    GameRuntime runtime(rules);

    // Execute game
    runtime.run();

    // Check game state
    const auto& gameState = runtime.getGameState();
    EXPECT_EQ(gameState.size(), 2);  // Should have 2 variables
}

// ============================================================================
// LOBBY TO GAME FLOW TESTS
// ============================================================================

TEST_F(Phase1IntegrationTest, HostCanStartGame)
{
    // Alice creates lobby
    std::string lobbyID = createLobby(ALICE_ID, "Alice");
    ASSERT_FALSE(lobbyID.empty());

    // Alice starts game (as host)
    auto responses = startGame(ALICE_ID, "Alice");

    // Should notify Alice
    ASSERT_GT(responses.size(), 0);
    EXPECT_EQ(responses[0].clientID, ALICE_ID);
    EXPECT_EQ(responses[0].message.type, MessageType::JoinGame);
}

TEST_F(Phase1IntegrationTest, NonHostCannotStartGame)
{
    // Alice creates lobby
    std::string lobbyID = createLobby(ALICE_ID, "Alice");
    ASSERT_FALSE(lobbyID.empty());

    // Bob joins
    joinLobby(BOB_ID, "Bob", lobbyID);

    // Bob tries to start game (should fail - not host)
    auto responses = startGame(BOB_ID, "Bob");

    // Should get error
    ASSERT_GT(responses.size(), 0);
    EXPECT_EQ(responses[0].message.type, MessageType::Error);

    auto& errorData = std::get<ErrorMessage>(responses[0].message.data);
    EXPECT_NE(errorData.reason.find("host"), std::string::npos);
}

TEST_F(Phase1IntegrationTest, GameNotifiesAllPlayers)
{
    // Alice creates lobby
    std::string lobbyID = createLobby(ALICE_ID, "Alice");

    // Bob and Carol join
    joinLobby(BOB_ID, "Bob", lobbyID);
    joinLobby(CAROL_ID, "Carol", lobbyID);

    // Alice starts game
    auto responses = startGame(ALICE_ID, "Alice");

    // Should notify all 3 players
    EXPECT_EQ(responses.size(), 3);

    // Check all client IDs are present
    bool foundAlice = false, foundBob = false, foundCarol = false;
    for (const auto& resp : responses) {
        if (resp.clientID == ALICE_ID) foundAlice = true;
        if (resp.clientID == BOB_ID) foundBob = true;
        if (resp.clientID == CAROL_ID) foundCarol = true;
        EXPECT_EQ(resp.message.type, MessageType::JoinGame);
    }

    EXPECT_TRUE(foundAlice);
    EXPECT_TRUE(foundBob);
    EXPECT_TRUE(foundCarol);
}

TEST_F(Phase1IntegrationTest, CannotStartGameTwice)
{
    // Alice creates lobby
    std::string lobbyID = createLobby(ALICE_ID, "Alice");

    // Alice starts game (first time - should succeed)
    auto responses1 = startGame(ALICE_ID, "Alice");
    ASSERT_GT(responses1.size(), 0);

    // Alice tries to start again (should fail)
    auto responses2 = startGame(ALICE_ID, "Alice");

    // Should get error
    ASSERT_GT(responses2.size(), 0);
    EXPECT_EQ(responses2[0].message.type, MessageType::Error);

    auto& errorData = std::get<ErrorMessage>(responses2[0].message.data);
    EXPECT_NE(errorData.reason.find("already"), std::string::npos);
}

TEST_F(Phase1IntegrationTest, CannotStartGameWithoutLobby)
{
    // Alice tries to start game without being in a lobby
    auto responses = startGame(ALICE_ID, "Alice");

    // Should get error
    ASSERT_GT(responses.size(), 0);
    EXPECT_EQ(responses[0].message.type, MessageType::Error);

    auto& errorData = std::get<ErrorMessage>(responses[0].message.data);
    EXPECT_NE(errorData.reason.find("lobby"), std::string::npos);
}

// ============================================================================
// MULTIPLE LOBBIES / GAMES TESTS
// ============================================================================

TEST_F(Phase1IntegrationTest, MultipleLobbyGamesIndependent)
{
    // Alice creates lobby 1
    std::string lobby1 = createLobby(ALICE_ID, "Alice");

    // Bob creates lobby 2
    std::string lobby2 = createLobby(BOB_ID, "Bob");

    EXPECT_NE(lobby1, lobby2);  // Different lobbies

    // Alice starts game in lobby 1
    auto responses1 = startGame(ALICE_ID, "Alice");
    ASSERT_GT(responses1.size(), 0);
    EXPECT_EQ(responses1[0].clientID, ALICE_ID);

    // Bob starts game in lobby 2
    auto responses2 = startGame(BOB_ID, "Bob");
    ASSERT_GT(responses2.size(), 0);
    EXPECT_EQ(responses2[0].clientID, BOB_ID);

    // Both should succeed independently
    EXPECT_EQ(responses1[0].message.type, MessageType::JoinGame);
    EXPECT_EQ(responses2[0].message.type, MessageType::JoinGame);
}

// ============================================================================
// EDGE CASES
// ============================================================================

TEST_F(Phase1IntegrationTest, EmptyGameRulesExecuteSuccessfully)
{
    // Create empty game
    std::vector<std::unique_ptr<ast::Statement>> statements;
    GameRules rules{std::span(statements)};

    GameRuntime runtime(rules);

    // Should not crash
    EXPECT_NO_THROW(runtime.run());

    const auto& gameState = runtime.getGameState();
    EXPECT_EQ(gameState.size(), 0);  // No variables
}

TEST_F(Phase1IntegrationTest, GameSessionCannotStartTwice)
{
    std::vector<std::unique_ptr<ast::Statement>> statements;
    statements.push_back(
        ast::makeAssignment(
            ast::makeVariable(Name{"value"}),
            ast::makeConstant(Value{Integer{42}})
        )
    );

    GameRules rules{std::span(statements)};
    std::vector<LobbyMember> players = {
        LobbyMember{ALICE_ID, LobbyRole::Host, true}
    };

    GameSession session("test", rules, players);

    // Start once
    session.start();
    EXPECT_TRUE(session.isFinished());

    // Try to start again - should be no-op
    session.start();
    EXPECT_TRUE(session.isFinished());  // Still finished
}

// ============================================================================
// INTEGRATION WITH EXISTING LOBBY TESTS
// ============================================================================

TEST_F(Phase1IntegrationTest, LobbyStatePersistsAfterGameStarts)
{
    // Alice creates lobby
    std::string lobbyID = createLobby(ALICE_ID, "Alice");
    joinLobby(BOB_ID, "Bob", lobbyID);

    // Start game
    startGame(ALICE_ID, "Alice");

    // Get lobby state - lobby should still exist
    Message getLobbyMsg;
    getLobbyMsg.type = MessageType::GetLobbyState;
    getLobbyMsg.data = GetLobbyStateMessage{};

    auto responses = server->tick({ClientMessage{ALICE_ID, getLobbyMsg}});

    // Should still get lobby state
    ASSERT_GT(responses.size(), 0);
    EXPECT_EQ(responses[0].message.type, MessageType::LobbyState);
}
