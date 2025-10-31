#include <iostream>
#include "GameServer.h"
#include "Networking.h"
#include "Message.h"

GameServer::GameServer(std::shared_ptr<NetworkingInterface> networking)
    : m_networking(networking) {}


void GameServer::onMessageFromClient(int fromClientID, Message &message)
{
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

void GameServer::broadcastUpdate(int cycle)
{
    Message update{MessageType::UpdateCycle, UpdateCycleMessage{ cycle }};
    const std::vector<int> ids = m_networking->getConnectedClientIDs();

    std::cout << "[GameServer] update#" << cycle << " for clients: ";
    bool first = true;
    for (int id : ids) {
        if (!first) std::cout << ", ";
        std::cout << id;
        m_networking->sendMessageToClient(id, update);
        first = false;
    }
    std::cout << "\n";
}