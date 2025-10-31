#include <gtest/gtest.h>
#include "Game.h"
#include "Rules.h"


class CountingRuleInstance : public RuleInstance
{
    public:
        CountingRuleInstance(int& counter)
        : m_counter(counter), m_done(false) {}

        std::vector<RuleCommand> run(Game& game) override
        {
            m_counter++;
            m_done = true;
            return {};
        }

        bool isDone() override { return m_done; }

    private:
        int& m_counter;
        bool m_done;
};

class CountingRuleBlueprint : public RuleBlueprint
{
    public:
        CountingRuleBlueprint(int& counter)
        : m_counter(counter) {}

        std::shared_ptr<RuleInstance> create() override
        {
            return std::make_shared<CountingRuleInstance>(m_counter);
        }

    private:
        int& m_counter;
};


TEST(ForLoopRuleTest, executesInnerRuleNTimes)
{
    int runCount = 0;
    int loopIterations = 5;

    auto innerRule = std::make_shared<CountingRuleBlueprint>(runCount);
    auto loopRule = std::make_shared<ForLoopRuleBlueprint>(loopIterations, innerRule);

    std::vector<std::shared_ptr<RuleBlueprint>> rules = { loopRule };
    std::vector<Player> players;

    Game game(rules, players);

    while (game.getState() != Game::State::GameOver)
    {
        game.tick();
    }

    ASSERT_EQ(loopIterations, runCount);
}
