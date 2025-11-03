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

        virtual void sendToClient(uintptr_t toClientID, const Message &message) = 0;
        // virtual void sendMessageToServer(int toClientID, const Message &message) = 0;
        virtual std::vector<std::pair<uintptr_t, Message> > receiveFromClients() = 0;
        virtual std::vector<uintptr_t> getConnectedClientIDs() const = 0;
};

// TODO (if interface makes sense):
// class WebSocketNetworking : public NetworkingInterface {}
