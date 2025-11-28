#include "InputManager.h"
#include <stdexcept>
#include <sstream>

std::optional<String>
InputManager::getTextInput(String playerID, String prompt)
{
    auto response = findResponse(playerID, prompt);
    if (response) {
        return response;
    }

    if (!hasRequestedInput(playerID, prompt)) {
        addPendingRequest(GameMessage{GetTextInputMessage{playerID, prompt}});
    }

    return std::nullopt;
}

std::optional<String>
InputManager::getChoiceInput(String playerID, String prompt, const List<Value>& choices)
{
    auto response = findResponse(playerID, prompt);
    if (response) {
        return response;
    }

    if (!hasRequestedInput(playerID, prompt)) {
        addPendingRequest(GameMessage{GetChoiceInputMessage{playerID, prompt, choices}});
    }

    return std::nullopt;
}

std::optional<Integer>
InputManager::getRangeInput(String playerID, String prompt, Integer minValue, Integer maxValue)
{
    auto response = findResponse(playerID, prompt);
    if (response) {
        try {
            Integer value = Integer{std::stoi(response->value)};
            if (value < minValue || maxValue < value) {
                std::ostringstream oss;
                oss << "Range input " << value
                    << " is outside valid range (" << minValue << ", " << maxValue << ")";
                throw std::runtime_error(oss.str());
            }
            return value;
        } catch (const std::invalid_argument&) {
            throw std::runtime_error("Invalid range input, enter a number");
        }
    }

    if (!hasRequestedInput(playerID, prompt)) {
        addPendingRequest(GameMessage{GetRangeInputMessage{playerID, prompt, minValue, maxValue}});
    }

    return std::nullopt;
}

std::optional<String>
InputManager::getVoteInput(String playerID, String prompt, const List<Value>& choices)
{
    auto response = findResponse(playerID, prompt);
    if (response) {
        return response;
    }

    if (!hasRequestedInput(playerID, prompt)) {
        addPendingRequest(GameMessage{GetVoteInputMessage{playerID, prompt, choices}});
    }

    return std::nullopt;
}

void
InputManager::handleIncomingMessages(const std::vector<GameMessage>& messages)
{
    for (const auto& msg : messages) {
        if (const auto* textInput = std::get_if<TextInputMessage>(&msg.inner)) {
            m_responses[textInput->playerID][textInput->prompt] = textInput->input;
        }
        else if (const auto* choiceInput = std::get_if<ChoiceInputMessage>(&msg.inner)) {
            m_responses[choiceInput->playerID][choiceInput->prompt] = choiceInput->choice;
        }
        else if (const auto* rangeInput = std::get_if<RangeInputMessage>(&msg.inner)) {
            m_responses[rangeInput->playerID][rangeInput->prompt] =
                String{std::to_string(rangeInput->value.value)};
        }
        else if (const auto* voteInput = std::get_if<VoteInputMessage>(&msg.inner)) {
            m_responses[voteInput->playerID][voteInput->prompt] = voteInput->vote;
        }
    }
}

std::vector<GameMessage>
InputManager::getPendingRequests()
{
    return m_pendingRequests;
}

void
InputManager::clearPendingRequests()
{
    m_pendingRequests.clear();
}

void
InputManager::sendOutput(const String &message) {
    m_pendingOutputs.push_back(message.value);
}

std::vector<std::string>
InputManager::popPendingOutputs() {
    std::vector<std::string> out = std::move(m_pendingOutputs);
    m_pendingOutputs.clear();
    return out;
}

std::optional<String>
InputManager::findResponse(const String& playerID, const String& prompt) const
{
    auto playerIt = m_responses.find(playerID);
    if (playerIt == m_responses.end()) {
        return std::nullopt;
    }

    auto promptIt = playerIt->second.find(prompt);
    if (promptIt == playerIt->second.end()) {
        return std::nullopt;
    }

    return promptIt->second;
}

bool
InputManager::hasRequestedInput(const String& playerID, const String& prompt) const
{
    auto playerIt = m_sentRequests.find(playerID);
    if (playerIt == m_sentRequests.end()) {
        return false;
    }
    return playerIt->second.find(prompt) != playerIt->second.end();
}

void
InputManager::addPendingRequest(GameMessage request)
{
    if (const auto* textReq = std::get_if<GetTextInputMessage>(&request.inner)) {
        m_sentRequests[textReq->playerID].insert(textReq->prompt);
    }
    else if (const auto* choiceReq = std::get_if<GetChoiceInputMessage>(&request.inner)) {
        m_sentRequests[choiceReq->playerID].insert(choiceReq->prompt);
    }
    else if (const auto* rangeReq = std::get_if<GetRangeInputMessage>(&request.inner)) {
        m_sentRequests[rangeReq->playerID].insert(rangeReq->prompt);
    }
    else if (const auto* voteReq = std::get_if<GetVoteInputMessage>(&request.inner)) {
        m_sentRequests[voteReq->playerID].insert(voteReq->prompt);
    }

    m_pendingRequests.push_back(std::move(request));
}
