#include <format>
#include "Rules.h"
#include "Game.h"


/*
 *******************************************************
 *                     ForLoopRule                     *
 *******************************************************
 */


ForLoopRuleBlueprint::ForLoopRuleBlueprint(int iterations,
                                           std::shared_ptr<RuleBlueprint> innerRuleBlueprint)
: m_iterations(iterations)
, m_innerRuleBlueprint(innerRuleBlueprint) {}

std::shared_ptr<RuleInstance> ForLoopRuleBlueprint::create()
{
    return std::make_shared<ForLoopRuleInstance>(m_iterations, m_innerRuleBlueprint);
}

ForLoopRuleInstance::ForLoopRuleInstance(int iterations,
                                         std::shared_ptr<RuleBlueprint> innerRuleBlueprint)
: m_iterations(iterations)
, m_innerRuleBlueprint(innerRuleBlueprint)
, m_currentIteration(0) {}


std::vector<RuleCommand> ForLoopRuleInstance::run(Game &game)
{
    if (isDone())
    {
        return {};
    }

    std::vector<RuleCommand> commands;

    if (!m_currentRuleInstance)
    {
        std::shared_ptr<RuleInstance> ruleInstance = m_innerRuleBlueprint->create();
        m_currentRuleInstance = ruleInstance;
    }

    if (m_currentRuleInstance->isDone())
    {
        m_currentRuleInstance = nullptr;
        m_currentIteration++;
    }
    else
    {
        commands.push_back(RunRuleCommand{m_currentRuleInstance});
    }

    return commands;
}

bool ForLoopRuleInstance::isDone()
{
    return m_currentIteration == m_iterations;
}
