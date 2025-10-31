#include "Game.h"
#include "Rules.h"


Game::Game(std::vector<std::shared_ptr<RuleBlueprint>> ruleBlueprints,
           std::vector<Player> players)
: m_ruleBlueprints(ruleBlueprints)
, m_state(State::Init)
, m_currentRuleIndex(0)
{
    for (const auto& player : players)
    {
        m_players[player.id] = player;
    }
}


void Game::tick()
{
    switch (m_state)
    {
        case State::Init:
        {
            m_state = m_ruleBlueprints.empty() ? State::GameOver : State::InGame;
            break;
        }
        case State::InGame:
        {
            if (m_ruleExecutionStack.empty())
            {
                if (m_currentRuleIndex < m_ruleBlueprints.size())
                {
                    m_state = State::InGame;

                    // Create and run the next rule
                    std::shared_ptr<RuleBlueprint> ruleBlueprint = m_ruleBlueprints[m_currentRuleIndex];
                    std::shared_ptr<RuleInstance> ruleInstance = ruleBlueprint->create();
                    m_currentRuleIndex++;

                    runRule(ruleInstance);
                }
                else
                {
                    // All rules done
                    m_state = State::GameOver;
                }
            }
            else
            {
                // Run the current top rule
                std::shared_ptr<RuleInstance> currentRuleInstance = m_ruleExecutionStack.front();
                runRule(currentRuleInstance);
            }
            break;
        }
        case State::GameOver:
        {
            // Nothing to do
        }
    }
}

void Game::runRule(std::shared_ptr<RuleInstance> ruleInstance)
{
    if (m_ruleExecutionStack.empty()
        || m_ruleExecutionStack.front().get() != ruleInstance.get())
    {
        // This is a new rule to execute, push it to the stack
        m_ruleExecutionStack.push_front(ruleInstance);
    }

    std::vector<RuleCommand> commands = ruleInstance->run(*this);

    processRuleCommands(commands);

    if (ruleInstance->isDone())
    {
        m_ruleExecutionStack.pop_front();
    }
}

void Game::processRuleCommands(std::vector<RuleCommand>& commands)
{
    for (const RuleCommand& command : commands)
    {
        std::visit([this](auto&& cmd)
        {
            processRuleCommand(cmd);
        }, command);
    }
}

void Game::processRuleCommand(const RunRuleCommand& command)
{
    runRule(command.ruleInstance);
}

Game::State Game::getState()
{
    return m_state;
}

std::vector<Player> Game::getPlayers()
{
    std::vector<Player> players;
    players.reserve(m_players.size());

    for (const auto& [id, player] : m_players) {
        players.push_back(player);
    }

    return players;
}
