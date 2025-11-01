#include <iostream>
#include "GameServer.h"
#include "Networking.h"
#include "Message.h"

GameServer::GameServer(std::shared_ptr<NetworkingInterface> networking)
    : m_networking(networking)
{}

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