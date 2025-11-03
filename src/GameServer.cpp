#include <iostream>
#include "GameServer.h"
#include "Message.h"

std::vector<ClientMessage>
GameServer::tick(const std::vector<ClientMessage> &incomingMessages) {
    std::vector<ClientMessage> outgoingMessages;

    for(const auto& clientMsg : incomingMessages){
        switch (clientMsg.message.type)
        {
            case MessageType::JoinGame:
            {
                auto& joinData = std::get<JoinGameMessage>(clientMsg.message.data);
                std::cout << "JoinGame(\"" << joinData.playerName << "\")\n";

                Message response;
                response.type = MessageType::JoinGame;
                response.data = JoinGameMessage{joinData.playerName};

                outgoingMessages.push_back({clientMsg.clientID, response});
                continue;
            }
            case MessageType::UpdateCycle:
            {
                auto& data = std::get<UpdateCycleMessage>(clientMsg.message.data);
                std::cout << "UpdateCycle from client with cycle " << data.cycle << "\n";
                continue;
            }
            default:
            {
                std::cout << "Empty message" << "\n";
                continue;
            }
        }
    }
    return outgoingMessages;
}