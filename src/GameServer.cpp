#include <iostream>
#include "GameServer.h"
#include "Message.h"

GameServer::GameServer()
    : m_gameStarted(false), m_gameOver(false){}

std::vector<ClientMessage>
GameServer::tick(const std::vector<ClientMessage> &incomingMessages) {
    if(getState() == ServerState::InLobby){
        return handleLobbyMessages(incomingMessages);
    }else if(getState() == ServerState::InGame){
        return handleGameMessages(incomingMessages);
    }
    return {};
}

std::vector<ClientMessage>
GameServer::handleLobbyMessages(const std::vector<ClientMessage> &incomingMessages) {
    std::vector<ClientMessage> outgoingMessages;

    for(const auto& clientMsg : incomingMessages){
        switch(clientMsg.message.type)
        {
            case MessageType::JoinLobby:
            {
                auto &joinData = std::get<JoinLobbyMessage>(clientMsg.message.data);
                std::cout << "[Lobby] Player {" << joinData.playerName <<
                          "} joined (clientID = " << clientMsg.clientID << ")\n";

                Message response;
                response.type = MessageType::JoinLobby;
                response.data = JoinLobbyMessage{joinData.playerName};
                outgoingMessages.push_back({clientMsg.clientID, response});

                auto lobbyStateMessages = broadcastLobbyState();
                outgoingMessages.insert(
                        outgoingMessages.end(),
                        lobbyStateMessages.begin(),
                        lobbyStateMessages.end()
                );
                break;
            }
            case MessageType::Empty:
                break;
            case MessageType::JoinGame:
                break;
        }
    }
    return outgoingMessages;
}

std::vector<ClientMessage>
GameServer::handleGameMessages(const std::vector<ClientMessage> &incomingMessages) {
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
                break;
            }
            case MessageType::UpdateCycle:
            {
                auto& data = std::get<UpdateCycleMessage>(clientMsg.message.data);
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

    // TODO: process game logic with GameEngine

    return outgoingMessages;
}

GameServer::ServerState
GameServer::getState() const {
    if(m_gameStarted) return ServerState::InGame;
    if(m_gameOver) return ServerState::GameOver;
    return ServerState::InLobby;
}

std::vector<ClientMessage>
GameServer::broadcastLobbyState() {

    return {};
}