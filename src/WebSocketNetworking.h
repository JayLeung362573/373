#pragma once

#include "Networking.h"
#include "Server.h" // From nsumner/web-socket-networking
#include "Client.h" // From nsumner/web-socket-networking

#include <string>

#include <memory>
#include <unordered_map>
#include <deque>
#include <iostream>

class GameServer;
class GameClient;

class WebSocketNetworking : public NetworkingInterface, public std::enable_shared_from_this<WebSocketNetworking>
{
public:
    WebSocketNetworking(unsigned short port, const std::string& htmlPath);
    ~WebSocketNetworking() = default;

    void startServer();
    std::vector<std::pair<int, std::string>> update();

    void sendMessageToClient(int toClientID, Message& message) override;
    void sendMessageToServer(int fromClientID, Message& message) override;

    void setServer(std::shared_ptr<GameServer> server);

    std::vector<int> getConnectedClientIDs() const override;
private:
    std::unique_ptr<networking::Server> net_server;
    std::shared_ptr<GameServer> m_server;
    std::unordered_map<int, std::shared_ptr<GameClient>> m_clients;
    std::unordered_map<int, networking::Connection> m_connections;

    bool has_server_started = false;
};