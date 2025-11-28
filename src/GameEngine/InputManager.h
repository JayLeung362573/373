#pragma once

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <variant>

#include "Types.h"
#include "GameMessage.h"


class InputManager {
public:
    InputManager() = default;

    std::optional<String> getTextInput(String playerID, String prompt);
    std::optional<String> getChoiceInput(String playerID, String prompt, const List<Value>& choices);
    std::optional<Integer> getRangeInput(String playerID, String prompt, Integer minValue, Integer maxValue);
    std::optional<String> getVoteInput(String playerID, String prompt, const List<Value>& choices);

    void handleIncomingMessages(const std::vector<GameMessage>& messages);
    std::vector<GameMessage> getPendingRequests();
    void clearPendingRequests();

    void sendOutput(const String& message);
    std::vector<std::string> popPendingOutputs();

private:
    std::optional<String> findResponse(const String& playerID, const String& prompt) const;
    bool hasRequestedInput(const String& playerID, const String& prompt) const;
    void addPendingRequest(GameMessage request);

private:
    std::unordered_map<String, std::unordered_map<String, String>> m_responses;
    std::vector<GameMessage> m_pendingRequests;
    std::unordered_map<String, std::unordered_set<String>> m_sentRequests;
    std::vector<std::string> m_pendingOutputs;
};
