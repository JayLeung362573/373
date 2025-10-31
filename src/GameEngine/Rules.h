#pragma once

#include <memory>
#include <vector>
#include <variant>

class Game;
struct Player;
class RuleInstance;


struct RunRuleCommand
{
    std::shared_ptr<RuleInstance> ruleInstance;
};

using RuleCommand = std::variant<RunRuleCommand>;


class RuleInstance
{
    public:
        virtual ~RuleInstance() = default;
        virtual std::vector<RuleCommand> run(Game &game) = 0;
        virtual bool isDone() = 0;
};


class RuleBlueprint
{
    public:
        virtual ~RuleBlueprint() = default;
        virtual std::shared_ptr<RuleInstance> create() = 0;
};


/*
 *******************************************************
 *                    ForLoopRule                      *
 *******************************************************
 */

class ForLoopRuleBlueprint : public RuleBlueprint
{
    public:
        ForLoopRuleBlueprint(int iterations,
                             std::shared_ptr<RuleBlueprint> innerRuleBlueprint);

        std::shared_ptr<RuleInstance> create() override;

    private:
        int m_iterations;
        std::shared_ptr<RuleBlueprint> m_innerRuleBlueprint;
};


class ForLoopRuleInstance : public RuleInstance
{
    public:
        ForLoopRuleInstance(int iterations,
                            std::shared_ptr<RuleBlueprint> innerRuleBlueprint);

        std::vector<RuleCommand> run(Game &game);
        bool isDone();

    private:
        int m_iterations;
        std::shared_ptr<RuleBlueprint> m_innerRuleBlueprint;

        int m_currentIteration;
        std::shared_ptr<RuleInstance> m_currentRuleInstance;
};
