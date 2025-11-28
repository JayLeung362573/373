#include "GameServer.h"
#include "WebSocketNetworking.h"
#include "NetworkManager.h"

#include <thread>
#include <chrono>
#include <set>



int main() {
    auto networking = std::make_shared<WebSocketNetworking>(8080, "../src/lib/test.html");

    auto server = std::make_shared<GameServer>();

    NetworkManager manager(networking, server);

    networking->startServer();

    while (true) {
        networking->update();

        manager.processNewConnections();

        manager.processIncomingMessages();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}