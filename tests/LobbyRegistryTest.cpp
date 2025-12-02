#include <gtest/gtest.h>
#include "Lobby.h"
#include "LobbyRegistry.h"

// Individual Lobby Tests
TEST(LobbyTest, CreateEmptyLobby) {
    Lobby lobby(LobbyID {"lobby_0"}, GameType::Default, ClientID{1}, "EmptyLobby", "HostName"); // Create lobby, assign player with ClientID 1 as host
    EXPECT_FALSE(lobby.hasPlayer(ClientID{2}));
    EXPECT_TRUE(lobby.hasPlayer(ClientID{1}));
    EXPECT_TRUE(lobby.getHostID() == 1);
    EXPECT_EQ(lobby.getPlayerCount(), 1);
}

TEST(LobbyTest, MultiplePlayerLobby) {
    Lobby lobby(LobbyID {"lobby_1"}, GameType::Default, ClientID{1}, "TestLobby", "player1"); // Create lobby, assign player with ClientID 1 as host
    EXPECT_FALSE(lobby.hasPlayer(ClientID{2}));
    EXPECT_TRUE(lobby.hasPlayer(ClientID{1}));

    bool inserted = lobby.insertPlayer(ClientID{2}, "player2", LobbyRole::Player);
    EXPECT_TRUE(inserted);
    EXPECT_TRUE(lobby.hasPlayer(ClientID{2}));

    EXPECT_EQ(lobby.getPlayerCount(), 2);
}

TEST(LobbyTest, PreventDuplicateInsert) {
    Lobby lobby(LobbyID {"lobby_2"}, GameType::Default, ClientID{1}, "DuplicateLobby", "Host");
    EXPECT_TRUE(lobby.insertPlayer(ClientID{2}, "P2"));
    EXPECT_FALSE(lobby.insertPlayer(ClientID{2}, "P2")); // Duplicate player should not be inserted
    EXPECT_EQ(lobby.getPlayerCount(), 2); // Should remain the same non-duplicated size
}

TEST(LobbyTest, DeletePlayerRemovesThem) {
    Lobby lobby(LobbyID {"lobby_3"}, GameType::Default, 1, "DeleteTest", "Host");
    lobby.insertPlayer(ClientID{2}, "DeleteMe");
    EXPECT_TRUE(lobby.hasPlayer(ClientID{2}));
    EXPECT_EQ(lobby.getPlayerCount(), 2);

    lobby.deletePlayer(ClientID{2});
    EXPECT_FALSE(lobby.hasPlayer(ClientID{2}));
    EXPECT_EQ(lobby.getPlayerCount(), 1);
}

TEST(LobbyTest, IsFullPreventsInsert) {
    Lobby lobby(LobbyID {"lobby_4"}, GameType::Default, 1, "FullLobby", "Host");

    // Fill up to maxPlayers (10)
    size_t max_players_size = lobby.getInfo().maxPlayers; // Maybe have a getter that access the max size directly (it seems like a reusable variable)

    for (int i = 0; i < max_players_size - 1; ++i) {
        EXPECT_TRUE(lobby.insertPlayer(100 + i, "Filler_" + std::to_string(i)));
    }

    EXPECT_TRUE(lobby.isFull());
    EXPECT_EQ(lobby.getPlayerCount(), max_players_size); // plus the host
    EXPECT_FALSE(lobby.insertPlayer(ClientID {max_players_size * 100 - 1}, "Overflow")); // Cannot insert into full lobby
}

// LobbyRegistry Tests
TEST(LobbyRegistryTest, JoinEmptyLobbyCreatesNew) {
    LobbyRegistry registry;
    Lobby* lobby = registry.createLobby(ClientID{1}, GameType::Default, "Bob's Game", "Bob");

    ASSERT_NE(lobby, nullptr);
    EXPECT_EQ(lobby->getInfo().lobbyName, "Bob's Game");
    EXPECT_TRUE(lobby->hasPlayer(ClientID{1}));
}

TEST(LobbyRegistryTest, MovePlayerToExistingLobby) {
    LobbyRegistry registry;
    Lobby* lobby = registry.createLobby(ClientID{1}, GameType::Default, "Chess Match", "player1");
    ASSERT_NE(lobby, nullptr);
    LobbyID lobbyID = lobby->getInfo().lobbyID;

    Lobby* existingLobby = registry.joinLobby(ClientID{2}, lobbyID, "player2");
    ASSERT_NE(existingLobby, nullptr);
    EXPECT_EQ(lobby, existingLobby);
    EXPECT_TRUE(existingLobby->hasPlayer(ClientID{2}));
}

TEST(LobbyRegistryTest, MovePlayerToNonExistentLobbyFails) {
    LobbyRegistry registry;
    bool joined = registry.joinLobby(ClientID{1}, LobbyID{"invalid_lobby_id"}, "invalidPlayer");
    EXPECT_FALSE(joined);
}

TEST(LobbyRegistryTest, RemovePlayerFromAllLobbies) {
    LobbyRegistry registry;
    Lobby* lobby1 = registry.createLobby(ClientID{1}, GameType::Default, "Test1", "H1");
    Lobby* lobby2 = registry.createLobby(ClientID{2}, GameType::Default, "Test2", "H2");
    ASSERT_NE(lobby1, nullptr);
    ASSERT_NE(lobby2, nullptr);

    LobbyID id1 = lobby1->getInfo().lobbyID;
    LobbyID id2 = lobby2->getInfo().lobbyID;

    Lobby* joined1 = registry.joinLobby(ClientID{3}, LobbyID{id1}, "player3");
    ASSERT_NE(joined1, nullptr);
    EXPECT_TRUE(joined1->hasPlayer(ClientID{3}));

    Lobby* joined2 = registry.joinLobby(ClientID{3}, LobbyID{id2}, "player4");
    ASSERT_EQ(joined2, nullptr);
    EXPECT_FALSE(lobby2->hasPlayer(ClientID{3}));

    registry.leaveLobby(ClientID{3});

    Lobby* lobby1After = registry.getLobby(id1);
    ASSERT_NE(lobby1After, nullptr);
    EXPECT_FALSE(lobby1After->hasPlayer(ClientID{3}));
    EXPECT_TRUE(lobby1After->hasPlayer(ClientID{1}));
}

TEST(LobbyRegistryTest, FindLobbyForClientReturnsCorrectLobby) {
    LobbyRegistry registry;
    Lobby* lobby = registry.createLobby(ClientID{1}, GameType::Default, "FindTest", "finder");
    LobbyID id1 = lobby->getInfo().lobbyID;

    registry.joinLobby(ClientID{5}, LobbyID{id1}, "joiner");
    auto result = registry.findLobbyForClient(5);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), id1);
}

TEST(LobbyRegistryTest, FindLobbyForNonexistentClientReturnsNullopt) {
    LobbyRegistry registry;
    registry.createLobby(ClientID{1}, GameType::Default, "FindNone", "noExistingPlayer");

    auto result = registry.findLobbyForClient(99);
    EXPECT_FALSE(result.has_value());
}


