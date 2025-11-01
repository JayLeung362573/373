#include <gtest/gtest.h>
#include "Networking.h"
#include "Message.h"
#include <memory>
TEST(InMemoryNetworkingTest, SimulateClientMessage){
    auto network = std::make_shared<InMemoryNetworking>();
    int clientID = 0;
    network->addConnectedClient(clientID);

    Message msg;
    msg.type = MessageType::JoinGame;
    msg.data = JoinGameMessage{"TestPlayer 0"};
    network->simulateClientMessage(clientID, msg);

    auto receivedMessages = network->receiveFromClients();
    ASSERT_EQ(receivedMessages.size(), 1);
    ASSERT_EQ(receivedMessages[0].first, clientID);
    ASSERT_EQ(receivedMessages[0].second.type, MessageType::JoinGame);
}