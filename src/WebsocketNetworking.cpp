#include "WebSocketNetworking.h"
#include "GameServer.h"
#include "GameClient.h"
#include <fstream>

#include "MessageTranslator.h"

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    return {std::istreambuf_iterator<char>(file), { }};
}

WebSocketNetworking::WebSocketNetworking(unsigned short port, const std::string& htmlPath)
{
    std::string httpPage = readFile(htmlPath);

    // Create server with callbacks
    net_server = std::make_unique<networking::Server>(
        port,
        httpPage,
        [this](networking::Connection c) {
            int fromClientID = c.id;
            std::cout << "[WebSocket] Client connected (id=" << fromClientID << ")\n";

            if (!m_clients.contains(fromClientID) && m_server && has_server_started) {
                auto client = std::make_shared<GameClient>(fromClientID, shared_from_this());
                m_clients[fromClientID] = client;
            }

        },
        [this](networking::Connection c) {
            int fromClientID = c.id;
            std::cout << "[WebSocket] Client disconnected (id=" << fromClientID << ")\n";

            m_clients.erase(fromClientID);
        }
    );

    std::cout << "[WebSocket] Server initialized on port " << port << "\n";
}

void WebSocketNetworking::setServer(std::shared_ptr<GameServer> server) {
    m_server = server;
}

void WebSocketNetworking::startServer()
{
    std::cout << "[WebSocket] Server started.\n";
    has_server_started = true;
}

std::vector<std::pair<int, std::string>> WebSocketNetworking::update()
{
    try {
        net_server->update();

        auto messages = net_server->receive();
        
        std::vector<std::pair<int, std::string>> received;

        for (auto& msg : messages) {
            std::cout << "[WebSocket] Received: " << msg.text << " from client " << msg.connection.id << "\n";

            int fromClientID  = msg.connection.id ? msg.connection.id : 0;
            
            received.emplace_back(fromClientID, msg.text);
        }

        return received;
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << "\n";
    }

    // No messages, returning defaults
    return {};
}

void WebSocketNetworking::sendMessageToClient(int toClientID, Message& message)
{
    std::string payload = MessageTranslator::serialize(message); // This is temporarily, hopefully I can decouple this further

    // Convert our message into network::Message compatible with the web-socket format to send over the network
    std::deque<networking::Message> out{
        networking::Message{networking::Connection{(uintptr_t)toClientID}, payload}
    };

    net_server->send(out);
}

void WebSocketNetworking::sendMessageToServer(int fromClientID, Message& message)
{
    // Called by GameClient
    if (m_server) {
        m_server->onMessageFromClient(fromClientID, message);
    }
}

// TODO: Return a list of currently connected client IDs.
std::vector<int> WebSocketNetworking::getConnectedClientIDs() const {
    std::vector<int> ids;
    for (const auto& pair : m_clients) {
        ids.push_back(pair.first);
    }
    return ids;
}