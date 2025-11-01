#include "Networking.h"
#include "GameClient.h"
#include "GameServer.h"
#include <iostream>
#include <utility>

void InMemoryNetworking::sendToClient(int toClientID, const Message &message)
{
    std::cout << "[InMemory] Send message to client \n";
    m_outgoingMessages[toClientID].push_back(message);
}

std::vector<std::pair<int, Message> > InMemoryNetworking::receiveFromClients(){
    auto messages = m_incomingMessages;
    m_incomingMessages.clear();
    return messages;
}

std::vector<int> InMemoryNetworking::getConnectedClientIDs() const{
    return m_connectedClientsIDs;
}
void InMemoryNetworking::simulateClientMessage(int fromClientID, const Message& message)
{
    std::cout << "[InMemory] Client " << fromClientID << " sending message\n";
    m_incomingMessages.push_back(std::make_pair(fromClientID, message));
}

void InMemoryNetworking::addConnectedClient(int clientID){
    std::cout << "[InMemory] Client " << clientID << " connected\n";
    m_connectedClientsIDs.push_back(clientID);
}

void InMemoryNetworking::removeConnectedClient(int clientID){
    auto toRemove = std::find(m_connectedClientsIDs.begin(), m_connectedClientsIDs.end(), clientID);
    if(toRemove != m_connectedClientsIDs.end()){
        m_connectedClientsIDs.erase(toRemove);
    }
    m_outgoingMessages.erase(clientID);
}

std::vector<Message> InMemoryNetworking::getMessagesForClient(int clientID){
    auto toGet = m_outgoingMessages.find(clientID);
    if(toGet != m_outgoingMessages.end()){
        auto messages = m_outgoingMessages[clientID];
        m_outgoingMessages[clientID].clear();
        return messages;
    }
    return std::vector<Message>();
}