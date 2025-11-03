#include <iostream>
#include "GameServer.h"
#include "Message.h"

void GameServer::getClientMessages(int fromClientID, const Message &message) {
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
}

std::vector<std::pair<int, Message> > GameServer::getOutgoingMessages() {
    std::vector<std::pair<int, Message> > messages = std::move(m_outgoingMessages);
    m_outgoingMessages = {};
    return messages;
}

void GameServer::tick() {
    for(const auto& [clientID, message] : m_incomingMessages){
        if(message.type == MessageType::JoinGame){ /// example of process JoinGame message
            Message response;
            /// setup response for each message type
//            response.type =
//            response.data =
            m_outgoingMessages.push_back({clientID, response});
        }
    }
    m_incomingMessages.clear();
}