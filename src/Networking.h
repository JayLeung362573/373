#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include "Message.h"

class GameServer;
class GameClient;

// Handles passing messages between clients and server
class NetworkingInterface
{
    public:
        virtual ~NetworkingInterface() = default;

        virtual void sendToClient(int toClientID, const Message &message) = 0;
        // virtual void sendMessageToServer(int toClientID, const Message &message) = 0;
        virtual std::vector<std::pair<int, Message> > receiveFromClients() = 0;
        virtual std::vector<int> getConnectedClientIDs() const = 0;
};

// Networking interface that passes messages between clients and server in memory
class InMemoryNetworking : public NetworkingInterface
{
    public:
        void sendToClient(int toClientID, const Message &message) override;
        std::vector<std::pair<int, Message> > receiveFromClients() override;
        std::vector<int> getConnectedClientIDs() const override;
        
        void simulateClientMessage(int fromClientID, const Message& message);
        void addConnectedClient(int clientID);
        void removeConnectedClient(int clientID);
        std::vector<Message> getMessagesForClient(int clientID);
    private:
        std::vector<int> m_connectedClientsIDs;
        std::vector<std::pair<int, Message> > m_incomingMessages;
        std::unordered_map<int, std::vector<Message> > m_outgoingMessages;
};

// TODO (if interface makes sense):
// class WebSocketNetworking : public NetworkingInterface {}
