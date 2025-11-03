#include "GameClient.h"
#include <iostream>


GameClient::GameClient(int clientID)
    : m_clientID(clientID) {}


Message GameClient::prepareMessageToServer(const Message& message) const{
    return message;
}


void GameClient::onMessageFromServer(Message& message)
{
    std::cout << "[GameClient " << m_clientID 
    << "] Received message from server\n";
    // TODO
}


uintptr_t GameClient::getClientID()
{
    return m_clientID;
}
