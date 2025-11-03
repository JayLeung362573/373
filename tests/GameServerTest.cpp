#include <gtest/gtest.h>
#include "GameServer.h"
#include "Message.h"

TEST(GameServerTest, SingleClientJoinGame){
    GameServer server;

    Message joinMsg{MessageType::JoinGame,
                    JoinGameMessage{{"player 1"}}
    };
    server.getClientMessages(1, joinMsg);

    server.tick();

    std::vector<ClientMessage> outgoingMsg = server.getOutgoingMessages();

    ASSERT_GT(outgoingMsg.size(), 0);
    ASSERT_EQ(outgoingMsg[0].clientID, 1);
    ASSERT_EQ(outgoingMsg[0].message.type, MessageType::JoinGame);

    auto& firstResponse = outgoingMsg[0];
    auto& responseMsg = std::get<JoinGameMessage>(firstResponse.message.data);
    ASSERT_EQ(responseMsg.playerName, "player 1");
}

TEST(GameServerTest, MultipleClientsJoinGame){
    GameServer server;
    server.getClientMessages(1, {MessageType::JoinGame, JoinGameMessage{"player 1"}});
    server.getClientMessages(2, {MessageType::JoinGame, JoinGameMessage{"player 2"}});
    server.getClientMessages(3, {MessageType::JoinGame, JoinGameMessage{"player 3"}});

    server.tick();

    std::vector<ClientMessage> outgoingMsg = server.getOutgoingMessages();

    ASSERT_EQ(outgoingMsg.size(), 3);
    bool foundClient1, foundClient2, foundClient3 = false;
    for(const auto& msg : outgoingMsg){
        if(msg.clientID == 1) foundClient1 = true;
        if(msg.clientID == 2) foundClient2 = true;
        if(msg.clientID == 3) foundClient3 = true;
    }
    ASSERT_TRUE(foundClient1);
    ASSERT_TRUE(foundClient2);
    ASSERT_TRUE(foundClient3);
}

