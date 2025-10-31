#include "GameClient.h"
#include "Networking.h"


GameClient::GameClient(int clientID, std::shared_ptr<NetworkingInterface> networking)
    : m_clientID(clientID)
    , m_networking(networking) {}


void GameClient::sendMessageToServer(Message& message)
{
    m_networking->sendMessageToServer(m_clientID, message);
}


void GameClient::onMessageFromServer(Message& message)
{
    // TODO
}


int GameClient::getClientID()
{
    return m_clientID;
}
