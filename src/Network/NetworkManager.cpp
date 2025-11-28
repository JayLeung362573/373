#include "NetworkManager.h"
#include <iostream>

NetworkManager::NetworkManager(std::shared_ptr<WebSocketNetworking> net, std::shared_ptr<GameServer> server)
        : m_networking(std::move(net))
        , m_server(std::move(server))
{}

void NetworkManager::processNewConnections(){
    auto currentClients = m_networking->getConnectedClientIDs();

    for (uintptr_t clientID : currentClients) {
        if (m_knownClients.find(clientID) == m_knownClients.end()) {
            std::cout << "[Main] New Client Detected: " << clientID << "\n";

            auto responses = m_server->showCurrentLobbies(clientID);

            // Process and Send
            for(auto& response : responses) {
                m_networking->sendToClient(response.clientID, response.message);
            }

            m_knownClients.insert(clientID);
        }
    }
}

void NetworkManager::processIncomingMessages(){
    auto incomingMessages = m_networking->receiveFromClients();

    std::vector<ClientMessage> clientMessages;
    for(auto& [clientID, message] : incomingMessages){
        std::cout << "[Network] Processing incoming messages" << '\n';
        clientMessages.push_back({clientID, message});
    }

    // process game logic in game server
    auto outgoingMessages = m_server->tick(clientMessages);

    // send outgoing processed gameServer messages
    for(const auto& clientMsg : outgoingMessages){
        m_networking->sendToClient(clientMsg.clientID, clientMsg.message);
    }
}

