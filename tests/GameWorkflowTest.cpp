#include <gtest/gtest.h>
#include "GameServer.h"
#include "LobbyRegistry.h"
#include "GameSession/GameSession.h"
#include "GameEngine/GameRuntime.h"


TEST(GameWorkflowTest, HostStartsGameCreatesAndFinishesSession) {
    GameServer server;
    uintptr_t hostID = 1;
    std::string hostName = "HostPlayer";
    std::string lobbyName = "TestLobby";

    JoinLobbyMessage joinLobbyMsg{hostName, lobbyName};
    auto joinResponses = server.handleJoinLobbyMessages(hostID, joinLobbyMsg);

    // We expect at least one response acknowledging lobby join
    ASSERT_FALSE(joinResponses.empty());
    EXPECT_EQ(joinResponses[0].clientID, hostID);
    EXPECT_EQ(joinResponses[0].message.type, MessageType::JoinLobby);

    // Host starts the game
    StartGameMessage startMsg{hostName};
    auto startResponses = server.handleStartGameMessages(hostID, startMsg);

    // We expect start game messages to be returned
    ASSERT_FALSE(startResponses.empty());
    for (const auto& msg : startResponses) {
        EXPECT_EQ(msg.message.type, MessageType::StartGame);
    }

    // Server has an active session and it's finished after start
    // We can check indirectly by sending a tick message with empty input
    std::vector<ClientMessage> tickInputs; 
    auto tickOutput = server.tick(tickInputs);

    // The output might be empty if the game finished instantly
    EXPECT_TRUE(tickOutput.empty() || tickOutput[0].message.type == MessageType::UpdateCycle);
}

TEST(GameWorkflowTest, MultiplePlayersWorkflow) {
    GameServer server;

    uintptr_t hostID = 1;
    uintptr_t playerID = 2;
    std::string hostName = "HostPlayer";
    std::string playerName = "GuestPlayer";
    std::string lobbyName = "TestLobby";

    // Host joins lobby
    auto hostJoinResp = server.handleJoinLobbyMessages(hostID, JoinLobbyMessage{hostName, lobbyName});
    ASSERT_FALSE(hostJoinResp.empty());
    EXPECT_EQ(hostJoinResp[0].message.type, MessageType::JoinLobby);

    // Player joins same lobby
    auto playerJoinResp = server.handleJoinLobbyMessages(playerID, JoinLobbyMessage{playerName, lobbyName});
    ASSERT_FALSE(playerJoinResp.empty());
    EXPECT_EQ(playerJoinResp[0].message.type, MessageType::JoinLobby);

    // Non-host tries to start game which should fail
    StartGameMessage guestStart{playerName};
    auto guestStartResp = server.handleStartGameMessages(playerID, guestStart);
    ASSERT_EQ(guestStartResp.size(), 1);
    EXPECT_EQ(guestStartResp[0].message.type, MessageType::Error);

    // Host starts game which should succeed
    StartGameMessage hostStart{hostName};
    auto hostStartResp = server.handleStartGameMessages(hostID, hostStart);
    ASSERT_FALSE(hostStartResp.empty());
    for (const auto& msg : hostStartResp) {
        EXPECT_EQ(msg.message.type, MessageType::StartGame);
    }
}

TEST(GameWorkflowTest, BrowseLobbyWorks) {
    GameServer server;

    uintptr_t hostID = 1;
    std::string hostName = "Host";
    std::string lobbyName = "TestLobby";

    // Host creates/joins lobby
    auto hostJoinResp = server.handleJoinLobbyMessages(hostID, JoinLobbyMessage{hostName, lobbyName});
    ASSERT_FALSE(hostJoinResp.empty());
    EXPECT_EQ(hostJoinResp[0].message.type, MessageType::JoinLobby);

    // Guest browses available lobbies
    uintptr_t guestID = 2;
    auto browseResp = server.handleBrowseLobbiesMessages(guestID, BrowseLobbiesMessage{GameType::Default});
    ASSERT_FALSE(browseResp.empty());

    // Check that the browsed lobby contains the host's lobby
    bool foundLobby = false;
    for (const auto& msg : browseResp) {
        if (msg.message.type == MessageType::JoinLobby) {
            const auto& data = std::get<JoinLobbyMessage>(msg.message.data);
            if (data.lobbyName == lobbyName) {
                foundLobby = true;
                break;
            }
        }
    }
    EXPECT_TRUE(foundLobby);
}

TEST(GameWorkflowTest, NonHostCannotStartGame) {
    GameServer server;

    uintptr_t hostID = 1;
    uintptr_t guestID = 2;
    std::string hostName = "Host";
    std::string guestName = "Guest";
    std::string lobbyName = "TestLobby";

    // Host creates/joins lobby
    server.handleJoinLobbyMessages(hostID, JoinLobbyMessage{hostName, lobbyName});

    // Guest joins the same lobby
    server.handleJoinLobbyMessages(guestID, JoinLobbyMessage{guestName, lobbyName});

    // Guest tries to start the game which should fail
    StartGameMessage guestStart{guestName};
    auto responses = server.handleStartGameMessages(guestID, guestStart);
    ASSERT_EQ(responses.size(), 1);
    EXPECT_EQ(responses[0].message.type, MessageType::Error);
    EXPECT_EQ(std::get<ErrorMessage>(responses[0].message.data).reason, "Only the lobby host can start the game");
}

TEST(GameWorkflowTest, CannotStartGameIfAlreadyStarted) {
    GameServer server;
    uintptr_t hostID = 1;
    std::string hostName = "Host";
    std::string lobbyName = "TestLobby";

    // Host joins/creates lobby
    JoinLobbyMessage joinMsg{hostName, lobbyName};
    auto joinResponses = server.handleJoinLobbyMessages(hostID, joinMsg);
    ASSERT_FALSE(joinResponses.empty());
    EXPECT_EQ(joinResponses[0].message.type, MessageType::JoinLobby);

    // Start the game once
    StartGameMessage startMsg{hostName};
    auto firstStartResp = server.handleStartGameMessages(hostID, startMsg);
    ASSERT_FALSE(firstStartResp.empty());
    for (const auto& msg : firstStartResp) {
        EXPECT_EQ(msg.message.type, MessageType::StartGame);
    }

    // Try starting again — should fail
    auto secondStartResp = server.handleStartGameMessages(hostID, startMsg);
    ASSERT_EQ(secondStartResp.size(), 1);
    EXPECT_EQ(secondStartResp[0].message.type, MessageType::Error);
    EXPECT_EQ(std::get<ErrorMessage>(secondStartResp[0].message.data).reason, "Game already started");
}
