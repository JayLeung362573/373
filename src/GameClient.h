#pragma once

#include <memory>
#include "Message.h"

class NetworkingInterface;

// Handles game application logic client-side
// Communicates with server via NetworkingInterface
class GameClient
{
    public:
        GameClient(int clientID);
        Message prepareMessageToServer(const Message& message) const;
        void onMessageFromServer(Message& message);
        int getClientID();

    private:
        int m_clientID;
};
