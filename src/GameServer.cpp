#include <iostream>
#include "GameServer.h"
#include "Message.h"

void GameServer::getClientMessages(uintptr_t fromClientID, const Message &message) {
    std::cout << "[server] Got message from client (id=" << fromClientID << ") : ";
    switch (message.type)
    {
        case MessageType::JoinGame:
        {
            auto& data = std::get<JoinGameMessage>(message.data);
            std::cout << "JoinGame(\"" << data.playerName << "\")\n";
            break;
        }
        case MessageType::UpdateCycle:
        {
            auto& data = std::get<UpdateCycleMessage>(message.data);
            std::cout << "UpdateCycle from client with cycle " << data.cycle << "\n";
            break;
        }
        default:
        {
            std::cout << "Empty message" << "\n";
            break;
        }
    }
    m_incomingMessages.push_back({fromClientID, message});
}

std::vector<ClientMessage> GameServer::getOutgoingMessages() {
    std::vector<ClientMessage> messages = std::move(m_outgoingMessages);
    m_outgoingMessages = {};
    return messages;
}

void GameServer::tick() {
    for(const auto& clientMsg : m_incomingMessages){
        if(clientMsg.message.type == MessageType::JoinGame){ /// example of process JoinGame message
            auto& joinData = std::get<JoinGameMessage>(clientMsg.message.data);

            Message response;
            response.type = MessageType::JoinGame;
            response.data = JoinGameMessage{joinData.playerName};

            m_outgoingMessages.push_back({clientMsg.clientID, response});
        }
    }
    m_incomingMessages.clear();
}