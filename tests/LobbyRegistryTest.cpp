#include <gtest/gtest.h>
#include "Lobby.h"
#include "LobbyRegistry.h"

namespace {
LobbyMember makeMember(ClientID id, const std::string& name, LobbyRole role) {
    LobbyMember member;
    member.clientID = id;
    member.name = name;
    member.role = role;
    member.ready = false;
    return member;
}
}

// Individual Lobby Tests
TEST(LobbyTest, CreateEmptyLobby) {
    Lobby lobby(LobbyID {"lobby_0"}, GameType::Default, makeMember(ClientID{1}, "HostName", LobbyRole::Host), "EmptyLobby");
    EXPECT_FALSE(lobby.hasPlayer(ClientID{2}));
    EXPECT_TRUE(lobby.hasPlayer(ClientID{1}));
    EXPECT_TRUE(lobby.getHostID() == 1);
    EXPECT_EQ(lobby.getPlayerCount(), 1);
}

TEST(LobbyTest, MultiplePlayerLobby) {
    Lobby lobby(LobbyID {"lobby_1"}, GameType::Default, makeMember(ClientID{1}, "player1", LobbyRole::Host), "TestLobby");
    EXPECT_FALSE(lobby.hasPlayer(ClientID{2}));
    EXPECT_TRUE(lobby.hasPlayer(ClientID{1}));

    bool inserted = lobby.insertPlayer(makeMember(ClientID{2}, "player2", LobbyRole::Player));
    EXPECT_TRUE(inserted);
    EXPECT_TRUE(lobby.hasPlayer(ClientID{2}));

    EXPECT_EQ(lobby.getPlayerCount(), 2);
}

TEST(LobbyTest, PreventDuplicateInsert) {
    Lobby lobby(LobbyID {"lobby_2"}, GameType::Default, makeMember(ClientID{1}, "Host", LobbyRole::Host), "DuplicateLobby");
    EXPECT_TRUE(lobby.insertPlayer(makeMember(ClientID{2}, "P2", LobbyRole::Player)));
    EXPECT_FALSE(lobby.insertPlayer(makeMember(ClientID{2}, "P2", LobbyRole::Player))); // Duplicate player should not be inserted
    EXPECT_EQ(lobby.getPlayerCount(), 2); // Should remain the same non-duplicated size
}

TEST(LobbyTest, DeletePlayerRemovesThem) {
    Lobby lobby(LobbyID {"lobby_3"}, GameType::Default, makeMember(ClientID{1}, "Host", LobbyRole::Host), "DeleteTest");
    lobby.insertPlayer(makeMember(ClientID{2}, "DeleteMe", LobbyRole::Player));
    EXPECT_TRUE(lobby.hasPlayer(ClientID{2}));
    EXPECT_EQ(lobby.getPlayerCount(), 2);

    lobby.deletePlayer(ClientID{2});
    EXPECT_FALSE(lobby.hasPlayer(ClientID{2}));
    EXPECT_EQ(lobby.getPlayerCount(), 1);
}

TEST(LobbyTest, IsFullPreventsInsert) {
    Lobby lobby(LobbyID {"lobby_4"}, GameType::Default, makeMember(ClientID{1}, "Host", LobbyRole::Host), "FullLobby");

    // Fill up to maxPlayers (10)
    size_t max_players_size = lobby.getInfo().maxPlayers; // Maybe have a getter that access the max size directly (it seems like a reusable variable)

    for (int i = 0; i < max_players_size - 1; ++i) {
        EXPECT_TRUE(lobby.insertPlayer(makeMember(100 + i, "Filler_" + std::to_string(i), LobbyRole::Player)));
    }

    EXPECT_TRUE(lobby.isFull());
    EXPECT_EQ(lobby.getPlayerCount(), max_players_size); // plus the host
    EXPECT_FALSE(lobby.insertPlayer(makeMember(ClientID {max_players_size * 100 - 1}, "Overflow", LobbyRole::Player))); // Cannot insert into full lobby
}

// LobbyRegistry Tests
TEST(LobbyRegistryTest, JoinEmptyLobbyCreatesNew) {
    LobbyRegistry registry;
    auto lobbyResult = registry.createLobby(makeMember(ClientID{1}, "Bob", LobbyRole::Host), GameType::Default, "Bob's Game");

    ASSERT_TRUE(lobbyResult.succeeded());
    EXPECT_EQ(lobbyResult.lobby->getInfo().lobbyName, "Bob's Game");
    EXPECT_TRUE(lobbyResult.lobby->hasPlayer(ClientID{1}));
}

TEST(LobbyRegistryTest, MovePlayerToExistingLobby) {
    LobbyRegistry registry;
    auto lobbyResult = registry.createLobby(makeMember(ClientID{1}, "player1", LobbyRole::Host), GameType::Default, "Chess Match");
    ASSERT_TRUE(lobbyResult.succeeded());
    LobbyID lobbyID = lobbyResult.lobby->getInfo().lobbyID;

    auto existingLobby = registry.joinLobby(makeMember(ClientID{2}, "player2", LobbyRole::Player), lobbyID);
    ASSERT_TRUE(existingLobby.succeeded());
    EXPECT_EQ(lobbyResult.lobby, existingLobby.lobby);
    EXPECT_TRUE(existingLobby.lobby->hasPlayer(ClientID{2}));
}

TEST(LobbyRegistryTest, MovePlayerToNonExistentLobbyFails) {
    LobbyRegistry registry;
    auto joined = registry.joinLobby(makeMember(ClientID{1}, "invalidPlayer", LobbyRole::Player), LobbyID{"invalid_lobby_id"});
    EXPECT_FALSE(joined.succeeded());
    EXPECT_EQ(joined.error, LobbyError::LobbyNotFound);
}

TEST(LobbyRegistryTest, RemovePlayerFromAllLobbies) {
    LobbyRegistry registry;
    auto lobby1 = registry.createLobby(makeMember(ClientID{1}, "H1", LobbyRole::Host), GameType::Default, "Test1");
    auto lobby2 = registry.createLobby(makeMember(ClientID{2}, "H2", LobbyRole::Host), GameType::Default, "Test2");
    ASSERT_TRUE(lobby1.succeeded());
    ASSERT_TRUE(lobby2.succeeded());

    LobbyID id1 = lobby1.lobby->getInfo().lobbyID;
    LobbyID id2 = lobby2.lobby->getInfo().lobbyID;

    auto joined1 = registry.joinLobby(makeMember(ClientID{3}, "player3", LobbyRole::Player), LobbyID{id1});
    ASSERT_TRUE(joined1.succeeded());
    EXPECT_TRUE(joined1.lobby->hasPlayer(ClientID{3}));

    auto joined2 = registry.joinLobby(makeMember(ClientID{3}, "player4", LobbyRole::Player), LobbyID{id2});
    ASSERT_FALSE(joined2.succeeded());
    EXPECT_FALSE(lobby2.lobby->hasPlayer(ClientID{3}));

    registry.leaveLobby(ClientID{3});

    Lobby* lobby1After = registry.getLobby(id1);
    ASSERT_NE(lobby1After, nullptr);
    EXPECT_FALSE(lobby1After->hasPlayer(ClientID{3}));
    EXPECT_TRUE(lobby1After->hasPlayer(ClientID{1}));
}

TEST(LobbyRegistryTest, FindLobbyForClientReturnsCorrectLobby) {
    LobbyRegistry registry;
    auto lobby = registry.createLobby(makeMember(ClientID{1}, "finder", LobbyRole::Host), GameType::Default, "FindTest");
    ASSERT_TRUE(lobby.succeeded());
    LobbyID id1 = lobby.lobby->getInfo().lobbyID;

    registry.joinLobby(makeMember(ClientID{5}, "joiner", LobbyRole::Player), LobbyID{id1});
    auto result = registry.findLobbyForClient(5);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), id1);
}

TEST(LobbyRegistryTest, FindLobbyForNonexistentClientReturnsNullopt) {
    LobbyRegistry registry;
    auto createResult = registry.createLobby(makeMember(ClientID{1}, "noExistingPlayer", LobbyRole::Host), GameType::Default, "FindNone");
    ASSERT_TRUE(createResult.succeeded());

    auto result = registry.findLobbyForClient(99);
    EXPECT_FALSE(result.has_value());
}

