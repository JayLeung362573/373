#pragma once

#include "Networking.h"
#include "Server.h" // From nsumner/web-socket-networking
#include "Client.h" // From nsumner/web-socket-networking

#include <string>
#include <memory>
#include <unordered_map>
#include <deque>
#include <iostream>
#include "Message.h"

class GameServer;
class GameClient;

class WebSocketNetworking : public NetworkingInterface, public std::enable_shared_from_this<WebSocketNetworking>
{
public:
    WebSocketNetworking(unsigned short port, const std::string& htmlPath);
    ~WebSocketNetworking() = default;

    void startServer();
    void update();

    void sendToClient(uintptr_t toClientID, const Message& message) override;
    std::vector<ClientMessage> receiveFromClients() override;
    std::vector<uintptr_t> getConnectedClientIDs() const override;
private:
    std::unique_ptr<networking::Server> net_server;
    std::unordered_map<uintptr_t, networking::Connection> m_connections;
    bool has_server_started = false;
    std::vector<ClientMessage> m_incomingMessages;
};