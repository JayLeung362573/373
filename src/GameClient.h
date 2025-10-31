#pragma once

#include <memory>
#include "Message.h"

class NetworkingInterface;

// Handles game application logic client-side
// Communicates with server via NetworkingInterface
class GameClient
{
    public:
        GameClient(int clientID, std::shared_ptr<NetworkingInterface> networking);

        void sendMessageToServer(Message& message);
        void onMessageFromServer(Message& message);

        int getClientID();

    private:
        int m_clientID;
        std::shared_ptr<NetworkingInterface> m_networking;
};
