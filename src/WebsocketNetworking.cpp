#include "WebSocketNetworking.h"
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

            m_connections[fromClientID] = c;
        },
        [this](networking::Connection c) {
            int fromClientID = c.id;
            std::cout << "[WebSocket] Client disconnected (id=" << fromClientID << ")\n";

            m_connections.erase(fromClientID);
        }
    );

    std::cout << "[WebSocket] Server initialized on port " << port << "\n";
}


void WebSocketNetworking::startServer()
{
    std::cout << "[WebSocket] Server started.\n";
    has_server_started = true;
}

std::vector<std::pair<int, Message>> WebSocketNetworking::receiveFromClients(){
    std::vector<std::pair<int, Message>> receivedMessages;
    try{
        net_server->update();
        auto messages = net_server->receive();
        for(auto& msg : messages){
            std::cout << "[WebSocket] Received: " << msg.text
                      << "from client " << msg.connection.id << "\n";
            int fromClientID;
            if(msg.connection.id){
                fromClientID = msg.connection.id;
            }else{
                fromClientID = 0;
            }
            Message translatedMsg = MessageTranslator::deserialize(msg.text);
            receivedMessages.emplace_back(fromClientID, translatedMsg);
        }
    }
    catch(const std::exception& e){
        std::cerr << "[WebSocket] Server error: " << e.what() << "\n";
    }
    return receivedMessages;
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

void WebSocketNetworking::sendToClient(int toClientID, const Message& message)
{
    std::string payload = MessageTranslator::serialize(message); // This is temporarily, hopefully I can decouple this further
    std::cout << "[WebSocket] Sending to client " << toClientID << ": " << payload << "\n";
    // Convert our message into network::Message compatible with the web-socket format to send over the network
    std::deque<networking::Message> out{
            networking::Message{networking::Connection{(uintptr_t)toClientID}, payload}
    };

    net_server->send(out);
}

// TODO: Return a list of currently connected client IDs.
std::vector<int> WebSocketNetworking::getConnectedClientIDs() const {
    std::vector<int> ids;
    ids.reserve(m_connections.size());
    for (const auto& [clientID, connection] : m_connections) {
        ids.push_back(clientID);
    }
    return ids;
}