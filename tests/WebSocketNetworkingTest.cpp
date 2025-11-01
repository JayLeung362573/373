#include <gtest/gtest.h>
#include <cstdio>
#include "WebSocketNetworking.h"
#include <memory>
#include <fstream>
#include "Message.h"

class WebSocketNetworkingTest : public ::testing::Test{
    const std::string testHtmlPath = "html_for_test.html";
    
    void SetUp() override{
        std::ofstream htmlFile(testHtmlPath);
        if(htmlFile.is_open()){
            htmlFile << "<html><body>test page</body></html>";
            htmlFile.close();
        }
    }

    void TearDown() override{
        std::remove(testHtmlPath.c_str());
    }
};

TEST_F(WebSocketNetworkingTest, ServerInitializationAndFakeTest){
    auto network = std::make_unique<WebSocketNetworking>(8080, "test.html");
    ASSERT_NE(network, nullptr); // constructor

    auto clients = network->getConnectedClientIDs();
    ASSERT_EQ(clients.size(), 0); // initial clients

    auto messages = network->receiveFromClients();
    ASSERT_EQ(messages.size(), 0); // initial messages

    ASSERT_NO_THROW(network->startServer());

    ASSERT_NO_THROW(network->update());

    Message testMsg {MessageType::JoinGame, JoinGameMessage{"test join game"}};
    ASSERT_NO_THROW(network->sendToClient(888, testMsg)); // send to a non-exist client

    messages = network->receiveFromClients();
    ASSERT_EQ(messages.size(), 0);
}