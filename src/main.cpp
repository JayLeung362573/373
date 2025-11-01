#include "Message.h"
#include "Networking.h"
#include "GameClient.h"
#include "GameServer.h"

#include "WebSocketNetworking.h"
#include "MessageTranslator.h"

#include <thread>
#include <chrono>

// simple demo of two clients sending a JoinGame message to the server
int main(int argc, char* argv[])
{
    bool useWebSocket = false;

    if (argc > 1 && std::string(argv[1]) == "--use-websocket") {
        useWebSocket = true;
        std::cout << "Using Web socket network" << "\n";
    }

    int CLIENT_1_ID = 100;
    int CLIENT_2_ID = 101;

    // Send JoinGame messages from clients to server
    Message msg1{ MessageType::JoinGame, JoinGameMessage{"joe"} };
    Message msg2{ MessageType::JoinGame, JoinGameMessage{"amy"} };
    
    if (useWebSocket) {
        auto networking = std::make_shared<WebSocketNetworking>(8080, "../test.html");
        auto server = std::make_unique<GameServer>(networking);

        networking->startServer();

        int cycle = 1;
        auto last = std::chrono::steady_clock::now();

        while (true) {
            networking->update();
            auto incomingMessages = networking->receiveFromClients();

            for(auto& [clientID, message] : incomingMessages){
                std::cout << "[Network] Processing incoming messages" << '\n';
                server->onMessageFromClient(clientID, message);
            }

            auto now = std::chrono::steady_clock::now();
            if (now - last >= std::chrono::seconds(1)) {
                auto clientIDs = networking->getConnectedClientIDs();
                Message updateMsg{MessageType::UpdateCycle, UpdateCycleMessage{cycle}};
                for(int clientID : clientIDs){
                    networking->sendToClient(clientID, updateMsg);
                }
                cycle++;
                last = now;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } else {
        // in-memory networking for test
        auto networking = std::make_shared<InMemoryNetworking>();
        auto server  = std::make_unique<GameServer>(networking);

        networking->addConnectedClient(CLIENT_1_ID);
        networking->addConnectedClient(CLIENT_2_ID);

        networking->simulateClientMessage(CLIENT_1_ID, msg1);
        networking->simulateClientMessage(CLIENT_2_ID, msg2);

        auto incomingMessages = networking->receiveFromClients();
        for(auto& [clientID, message] : incomingMessages) {
            server->onMessageFromClient(clientID, message);
        }
    }
    return 0;
}