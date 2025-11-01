#include <gtest/gtest.h>
#include "WebSocketNetworking.h"
#include <memory>

TEST(WebSocketNetworkingTest, ServerInitialization){
    auto network = std::make_unique<WebSocketNetworking>(8080, "test.html");
    ASSERT_NE(network, nullptr);

    auto clients = network->getConnectedClientIDs();
    ASSERT_EQ(clients.size(), 0);
}