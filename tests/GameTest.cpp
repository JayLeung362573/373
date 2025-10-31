#include <gtest/gtest.h>
#include "Game.h"
#include "Rules.h"


class MockRuleInstance : public RuleInstance
{
    public:
        bool done = false;

        std::vector<RuleCommand> run(Game& game) override
        {
            done = true;
            return {};
        }

        bool isDone() override { return done; }
};


class MockRuleBlueprint : public RuleBlueprint
{
    public:
        std::shared_ptr<RuleInstance> create() override
        {
            return std::make_shared<MockRuleInstance>();
        }
};


TEST(GameTest, initialStateIsInit)
{
    std::vector<std::shared_ptr<RuleBlueprint>> rules;
    std::vector<Player> players;

    Game game(rules, players);
    ASSERT_EQ(Game::State::Init, game.getState());
}


TEST(GameTest, initToInGameIfRulesExist)
{
    std::vector<std::shared_ptr<RuleBlueprint>> rules = {std::make_shared<MockRuleBlueprint>()};
    std::vector<Player> players;

    Game game(rules, players);
    game.tick();
    ASSERT_EQ(Game::State::InGame, game.getState());
}


TEST(GameTest, initToGameOverIfNoRules)
{
    std::vector<std::shared_ptr<RuleBlueprint>> rules;
    std::vector<Player> players;

    Game game(rules, players);
    game.tick();
    ASSERT_EQ(Game::State::GameOver, game.getState());
}


TEST(GameTest, inGameToGameOverWhenRuleDone)
{
    std::vector<std::shared_ptr<RuleBlueprint>> rules = {std::make_shared<MockRuleBlueprint>()};
    std::vector<Player> players;

    Game game(rules, players);

    game.tick();
    ASSERT_EQ( Game::State::InGame, game.getState());

    game.tick();
    ASSERT_EQ( Game::State::InGame, game.getState());

    game.tick();
    ASSERT_EQ(Game::State::GameOver, game.getState());
}


TEST(GameTest, runMultipleRules)
{
    std::vector<std::shared_ptr<RuleBlueprint>> rules = {std::make_shared<MockRuleBlueprint>(),
                                                         std::make_shared<MockRuleBlueprint>(),
                                                         std::make_shared<MockRuleBlueprint>()};
    std::vector<Player> players;

    Game game(rules, players);
    ASSERT_EQ( Game::State::Init, game.getState());

    for (auto i = 0; i < 4; i++)
    {
        game.tick();
        ASSERT_EQ( Game::State::InGame, game.getState());
    }

    game.tick();
    ASSERT_EQ(Game::State::GameOver, game.getState());
}
