#include "Message.h"
#include "Networking.h"
#include "GameClient.h"
#include "GameServer.h"

#include "WebSocketNetworking.h"
#include "MessageTranslator.h"

#include <thread>
#include <chrono>
#include <iostream>

// Simple simulation of Phase 1 workflow: Lobby → Game
void runSimpleSimulation()
{
    std::cout << "\n========================================\n";
    std::cout << "  PHASE 1 SIMULATION: Lobby → Game\n";
    std::cout << "========================================\n\n";

    auto server = std::make_unique<GameServer>();

    const uintptr_t ALICE_ID = 100;
    const uintptr_t BOB_ID = 200;
    const uintptr_t CAROL_ID = 300;

    // ========== STEP 1: Alice creates a lobby ==========
    std::cout << "STEP 1: Alice creates a lobby\n";
    std::cout << "------------------------------\n";

    Message createLobbyMsg;
    createLobbyMsg.type = MessageType::JoinLobby;
    createLobbyMsg.data = JoinLobbyMessage{"Alice", ""};  // Empty name = create new

    auto responses = server->tick({ClientMessage{ALICE_ID, createLobbyMsg}});

    std::string lobbyID;
    if (!responses.empty() && std::holds_alternative<LobbyStateMessage>(responses[0].message.data)) {
        lobbyID = std::get<LobbyStateMessage>(responses[0].message.data).currentLobbyID;
        std::cout << "✓ Lobby created: '" << lobbyID << "'\n";
    }
    std::cout << "\n";

    // ========== STEP 2: Bob and Carol join ==========
    std::cout << "STEP 2: Bob and Carol join the lobby\n";
    std::cout << "-------------------------------------\n";

    Message bobJoinMsg;
    bobJoinMsg.type = MessageType::JoinLobby;
    bobJoinMsg.data = JoinLobbyMessage{"Bob", lobbyID};

    Message carolJoinMsg;
    carolJoinMsg.type = MessageType::JoinLobby;
    carolJoinMsg.data = JoinLobbyMessage{"Carol", lobbyID};

    server->tick({
        ClientMessage{BOB_ID, bobJoinMsg},
        ClientMessage{CAROL_ID, carolJoinMsg}
    });

    std::cout << "✓ Bob joined\n";
    std::cout << "✓ Carol joined\n";
    std::cout << "✓ Lobby now has 3 players\n\n";

    // ========== STEP 3: Alice (host) starts the game ==========
    std::cout << "STEP 3: Alice starts the game\n";
    std::cout << "------------------------------\n";

    Message startGameMsg;
    startGameMsg.type = MessageType::JoinGame;
    startGameMsg.data = JoinGameMessage{"Alice"};

    auto gameResponses = server->tick({ClientMessage{ALICE_ID, startGameMsg}});

    std::cout << "✓ Game started!\n";
    std::cout << "✓ Notified " << gameResponses.size() << " players\n\n";

    // ========== STEP 4: Show results ==========
    std::cout << "STEP 4: Game Results\n";
    std::cout << "--------------------\n";
    for (const auto& resp : gameResponses) {
        if (resp.message.type == MessageType::JoinGame) {
            std::string playerName;
            if (resp.clientID == ALICE_ID) playerName = "Alice";
            else if (resp.clientID == BOB_ID) playerName = "Bob";
            else if (resp.clientID == CAROL_ID) playerName = "Carol";

            auto& data = std::get<JoinGameMessage>(resp.message.data);
            std::cout << "  → " << playerName << " (ID: " << resp.clientID
                      << "): " << data.playerName << "\n";
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  SIMULATION COMPLETE!\n";
    std::cout << "========================================\n\n";
}

// simple demo of two clients sending a JoinGame message to the server
int main(int argc, char* argv[])
{
    // Show usage if no arguments
    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " [MODE]\n\n";
        std::cout << "Available modes:\n";
        std::cout << "  --simulate       Run Phase 1 simulation (Lobby → Game)\n";
        std::cout << "  --use-websocket  Start WebSocket server\n\n";
        std::cout << "Examples:\n";
        std::cout << "  " << argv[0] << " --simulate\n";
        std::cout << "  " << argv[0] << " --use-websocket\n";
        return 0;
    }

    // Check for simulation mode
    if (argc > 1 && std::string(argv[1]) == "--simulate") {
        runSimpleSimulation();
        return 0;
    }

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
        auto server = std::make_unique<GameServer>();

        networking->startServer();

        int cycle = 1;
        auto last = std::chrono::steady_clock::now();

        while (true) {
            networking->update();

            // pass incoming network messages to game server
            auto incomingMessages = networking->receiveFromClients();

            std::vector<ClientMessage> clientMessages;
            for(auto& [clientID, message] : incomingMessages){
                std::cout << "[Network] Processing incoming messages" << '\n';
                clientMessages.push_back({clientID, message});
            }

            // process game logic in game server
            auto outgoingMessages = server->tick(clientMessages);

            // send outgoing processed gameServer messages
            for(const auto& clientMsg : outgoingMessages){
                networking->sendToClient(clientMsg.clientID, clientMsg.message);
            }

            auto now = std::chrono::steady_clock::now();
            if (now - last >= std::chrono::seconds(1)) {
                auto clientIDs = networking->getConnectedClientIDs();
                Message updateMsg{MessageType::UpdateCycle, UpdateCycleMessage{cycle}};
                for(uintptr_t clientID : clientIDs){
                    networking->sendToClient(clientID, updateMsg);
                }
                cycle++;
                last = now;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    return 0;
}