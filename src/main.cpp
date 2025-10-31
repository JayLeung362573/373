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
        // Create the WebSocket networking instance
        std::shared_ptr<WebSocketNetworking> ws = std::make_shared<WebSocketNetworking>(8080, "../test.html");
        std::shared_ptr<NetworkingInterface> networking = ws;  // upcast automatically

        auto server  = std::make_shared<GameServer>(networking);
        auto client1 = std::make_shared<GameClient>(CLIENT_1_ID, networking);
        auto client2 = std::make_shared<GameClient>(CLIENT_2_ID, networking);

        ws->setServer(server);
        ws->startServer();

        client1->sendMessageToServer(msg1);
        client2->sendMessageToServer(msg2);

        int cycle = 1;
        auto last = std::chrono::steady_clock::now();

        while (true) {
            auto messages = ws->update();
            
            // Translating the received message from clients
            if (!messages.empty()) {
                for (auto& [clientID, payload] : messages) {
                    std::cout << "[Main] Raw payload from client " << clientID << ": " << payload << "\n";
                    
                    // Convert payload string -> Message object using MessageTranslator
                    Message gameMsg = MessageTranslator::deserialize(payload);
                    
                    // Pass it up to the game logic
                    ws->sendMessageToServer(clientID, gameMsg);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Expected output when launching test.html:
            // [WebSocket] Client connected (id=1360)
            // [WebSocket] Received: JoinGame: Hello server! from client 89129161327952
            // [server] Got message from client (id=1360) : JoinGame(" Hello server!")

            auto now = std::chrono::steady_clock::now();
            if (now - last >= std::chrono::seconds(1)) {
                server->broadcastUpdate(cycle++);
                last = now;
            }
        }
    } else {
        auto networking = std::make_shared<InMemoryNetworking>();

        auto server  = std::make_shared<GameServer>(networking);
        auto client1 = std::make_shared<GameClient>(CLIENT_1_ID, networking);
        auto client2 = std::make_shared<GameClient>(CLIENT_2_ID, networking);

        networking->setServer(server);
        networking->addClient(client1);
        networking->addClient(client2);

        client1->sendMessageToServer(msg1);
        client2->sendMessageToServer(msg2);

        // Expected output:
        // [server] Got message from client (ID=100) : JoinGame("joe")
        // [server] Got message from client (ID=101) : JoinGame("amy")
    }

    return 0;
}