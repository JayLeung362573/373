#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <deque>

#include "Player.h"
#include "Rules.h"


class Game
{
    public:
        enum class State
        {
            Init = 0,
            InGame,
            GameOver
        };

        Game(std::vector<std::shared_ptr<RuleBlueprint>> ruleBlueprints,
             std::vector<Player> players);

        void tick();

        State getState();
        std::vector<Player> getPlayers();

    private:
        void runRule(std::shared_ptr<RuleInstance> ruleInstance);

        void processRuleCommands(std::vector<RuleCommand>& commands);
        void processRuleCommand(const RunRuleCommand& command);

        std::vector<std::shared_ptr<RuleBlueprint>> m_ruleBlueprints;
        std::deque<std::shared_ptr<RuleInstance>> m_ruleExecutionStack;
        int m_currentRuleIndex;

        std::unordered_map<PlayerID, Player> m_players;

        State m_state;
};
