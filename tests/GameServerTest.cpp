#include <gtest/gtest.h>
#include "GameServer.h"
#include "Message.h"

TEST(GameServerTest, SingleClientJoinGame){
    GameServer server;

    std::vector<ClientMessage> incoming = {
            {1, {MessageType::StartGame, StartGameMessage{"player 1"}}}
    };

    std::vector<ClientMessage> outgoingMsg = server.tick(incoming);

    ASSERT_GT(outgoingMsg.size(), 0);
    ASSERT_EQ(outgoingMsg[0].clientID, 1);
    ASSERT_EQ(outgoingMsg[0].message.type, MessageType::StartGame);

    auto& firstResponse = outgoingMsg[0];
    auto& responseMsg = std::get<StartGameMessage>(firstResponse.message.data);
    ASSERT_EQ(responseMsg.playerName, "player 1");
}

TEST(GameServerTest, MultipleClientsJoinGame){
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

