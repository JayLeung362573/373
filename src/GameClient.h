#pragma once

#include <memory>
#include "Message.h"

class NetworkingInterface;

// Handles game application logic client-side
// Communicates with server via NetworkingInterface
class GameClient
{
public:
    GameClient(uintptr_t clientID);
    Message prepareMessageToServer(const Message& message) const;
    void onMessageFromServer(Message& message);
    uintptr_t getClientID();

private:
    uintptr_t m_clientID;
};
