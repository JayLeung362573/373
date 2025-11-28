#pragma once
#include <set>
#include <memory>
#include <vector>
#include "GameServer.h"
#include "WebSocketNetworking.h"

class NetworkManager{
public:
    NetworkManager(std::shared_ptr<WebSocketNetworking> network,
                   std::shared_ptr<GameServer> server);

    void processNewConnections();
    void processIncomingMessages();
private:
    std::shared_ptr<WebSocketNetworking> m_networking;
    std::shared_ptr<GameServer> m_server;
    std::set<uintptr_t> m_knownClients;
};