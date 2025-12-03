#include "NetworkManager.h"
#include <iostream>
#include <ranges>

// update: replace specific networking with its interface component for better encapsulation
NetworkManager::NetworkManager(std::shared_ptr<NetworkingInterface> net, std::shared_ptr<GameServer> server)
        : m_networking(std::move(net))
        , m_server(std::move(server))
{}

// Update: Use Sequence Collection to optimally manipulate our data so it's more readable and clear constraints
void NetworkManager::processNewConnections(){
    auto currentClients = m_networking->getConnectedClientIDs();
    auto hasClientIdFunc = [this](uintptr_t id) { return !m_knownClients.contains(id); };
    // Return viewables for cheap accesses
    auto filteredClientsView = currentClients 
                            | std::views::filter(hasClientIdFunc);

    for (uintptr_t clientID : filteredClientsView) {
        std::cout << "[Main] New Client Detected: " << clientID << "\n";

        auto responses = m_server->showCurrentLobbies(clientID);

        // Process and Send
        for(auto& response : responses) {
            m_networking->sendToClient(response.clientID, response.message);
        }

        m_knownClients.insert(clientID);
    }
}

void NetworkManager::processIncomingMessages(){
    auto incomingMessages = m_networking->receiveFromClients();

    // map incoming network payloads into ClientMessage objects using std
    std::vector<ClientMessage> clientMessages;
    clientMessages.reserve(incomingMessages.size());

    std::ranges::transform(
        incomingMessages,
        std::back_inserter(clientMessages),
        [](auto&& pair){
            return ClientMessage{pair.clientID, std::move(pair.message)};
        }
    );

    // process game logic in game server
    auto outgoingMessages = m_server->tick(clientMessages);

    // send outgoing processed gameServer messages
    for(const auto& clientMsg : outgoingMessages){
        m_networking->sendToClient(clientMsg.clientID, clientMsg.message);
    }
}