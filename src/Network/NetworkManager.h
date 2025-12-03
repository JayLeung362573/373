#pragma once
#include <set>
#include <memory>
#include <vector>
#include "GameServer.h"
#include "Networking.h"

// Replaced WebSocketNetworking to it's Interface component, for proper polymisphic layering and CIA secruity protocol against API misuse.
class NetworkManager{
public:
    NetworkManager(std::shared_ptr<NetworkingInterface> network,
                   std::shared_ptr<GameServer> server);

    void processNewConnections();
    void processIncomingMessages();
private:
    std::shared_ptr<NetworkingInterface> m_networking;
    std::shared_ptr<GameServer> m_server;
    std::set<uintptr_t> m_knownClients;
};
