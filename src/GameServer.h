#pragma once

#include <memory.h>
#include <unordered_map>
#include <vector>
#include "Message.h"


class NetworkingInterface;

// Handles game application logic server-side
// Communicates with clients via NetworkingInterface
class GameServer
{
    public:
        GameServer(std::shared_ptr<NetworkingInterface> networking);
        void onMessageFromClient(int fromClientID, Message &message);
        void broadcastUpdate(int cycle);

    private:
        std::shared_ptr<NetworkingInterface> m_networking;
};
