#include <gtest/gtest.h>
#include "GameEngine/GameRuntime.h"
#include "GameEngine/Rules.h"

TEST(GameRuntimeTest, RunExecutesStatementsAndUpdatesState) {
    // Setup simple rule: winner = "player1"
    std::vector<std::unique_ptr<ast::Statement>> stmts;
    stmts.push_back(
        ast::makeAssignment(
            ast::makeVariable(Name{"winner"}),
            ast::makeConstant(Value{String{"player1"}})
        )
    );

    ast::GameRules rules{std::move(stmts)};
    GameRuntime runtime(rules);

    runtime.run();

    const VariableMap& state = runtime.getGameState();
    Value* winnerVal = state.load(Name{"winner"});
    ASSERT_NE(winnerVal, nullptr); // sanity check
    EXPECT_EQ(winnerVal->asString(), String{"player1"});
}

// TODO: Revisit once IO implemented in GameInterpreter
// TEST(GameRuntimeTest, TickCompletesSimpleProgram) {
//     // Program: winner = "player1"
//     std::vector<std::unique_ptr<ast::Statement>> stmts;
//     stmts.push_back(
//         ast::makeAssignment(
//             ast::makeVariable(Name{"winner"}),
//             ast::makeConstant(Value{String{"player1"}})
//         )
//     );

//     ast::GameRules rules{std::span(stmts)};
//     GameRuntime runtime(rules);

//     // No input messages
//     std::vector<GameMessage> emptyMsgs;
//     auto outMsgs = runtime.tick(emptyMsgs);

//     EXPECT_TRUE(runtime.isFinished());
//     EXPECT_TRUE(outMsgs.empty());

//     const VariableMap& state = runtime.getGameState();
//     Value* winnerVal = state.load(Name{"winner"});
//     ASSERT_NE(winnerVal, nullptr); // sanity check
//     EXPECT_EQ(winnerVal->asString(), String{"player1"});

// }
