#pragma once

#include <memory.h>
#include <unordered_map>
#include <vector>
#include "Message.h"


class NetworkingInterface;

// Handles game application logic server-side
// Communicates with clients via NetworkingInterface
struct ClientMessage{
    uintptr_t clientID = 0;
    Message message;
};

class GameServer
{
public:
    void getClientMessages(uintptr_t fromClientID, const Message &message);
    std::vector<ClientMessage> getOutgoingMessages();
    void tick();
private:
    std::vector<ClientMessage> m_incomingMessages;
    std::vector<ClientMessage> m_outgoingMessages;
};
