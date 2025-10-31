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
        GameServer() = default;
        void onMessageFromClient(int fromClientID, Message &message);
    private:
    
};
