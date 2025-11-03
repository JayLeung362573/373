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
    void getClientMessages(int fromClientID, const Message &message);
    std::vector<std::pair<int, Message> > getOutgoingMessages();
    void tick();
private:
    std::vector<std::pair<int, Message> > m_incomingMessages;
    std::vector<std::pair<int, Message> > m_outgoingMessages;
};
