#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(MatchTest, MatchWithMultipleCandidates)
{
    InputManager inputManager;

    ast::StatementsBuilder programBuilder;
    ast::StatementsBuilder statementsBuilder;
    ast::MatchBuilder matchBuilder;

    auto statements = programBuilder
        .addStatement(
            matchBuilder
            .setTarget(ast::makeConstant(Value{String{"2"}}))
            .addCandidatePair(
                ast::makeConstant(Value{String{"1"}}),
                statementsBuilder.addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"x"}),
                        ast::makeConstant(Value{String{"Path 1 taken!"}})
                    )
                ).build()
            ).addCandidatePair(
                ast::makeConstant(Value{String{"2"}}),
                statementsBuilder.addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"x"}),
                        ast::makeConstant(Value{String{"Path 2 taken!"}})
                    )
                ).build()
            ).build()
        ).build();

    GameInterpreter interpreter(inputManager, Program{std::move(statements)});

    interpreter.execute();

    auto storedX = loadVariable(interpreter, Name{"x"});
    EXPECT_EQ(storedX.asString(), String{"Path 2 taken!"});
}

TEST(MatchTest, NoMatch)
{
    InputManager inputManager;

    ast::StatementsBuilder programBuilder;
    ast::StatementsBuilder statementsBuilder;
    ast::MatchBuilder matchBuilder;

    auto statements = programBuilder
        .addStatement(
            matchBuilder
            .setTarget(ast::makeConstant(Value{String{"2"}}))
            .addCandidatePair(
                ast::makeConstant(Value{String{"1"}}),
                statementsBuilder.addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"x"}),
                        ast::makeConstant(Value{String{"Path 1 taken!"}})
                    )
                ).build()
            ).build()
        ).build();

    GameInterpreter interpreter(inputManager, Program{std::move(statements)});

    interpreter.execute();

    EXPECT_THROW({
        // x shouldn't be defined because assignment statement shouldn't have executed
        loadVariable(interpreter, Name{"x"});
    }, std::runtime_error);
}


TEST(MatchTest, MultipleStatements)
{
    InputManager inputManager;

    ast::StatementsBuilder programBuilder;
    ast::StatementsBuilder statementsBuilder;
    ast::MatchBuilder matchBuilder;

    auto statements = programBuilder
        .addStatement(
            matchBuilder
            .setTarget(ast::makeConstant(Value{String{"1"}}))
            .addCandidatePair(
                ast::makeConstant(Value{String{"1"}}),
                statementsBuilder.addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"x"}),
                        ast::makeConstant(Value{String{"Path 1 taken!"}})
                    )
                ).addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"y"}),
                        ast::makeConstant(Value{String{"Path 1 taken!"}})
                    )
                ).build()
            ).build()
        ).build();

    GameInterpreter interpreter(inputManager, Program{std::move(statements)});

    interpreter.execute();

    auto storedX = loadVariable(interpreter, Name{"x"});
    auto storedY = loadVariable(interpreter, Name{"y"});
    EXPECT_EQ(storedX.asString(), String{"Path 1 taken!"});
    EXPECT_EQ(storedY.asString(), String{"Path 1 taken!"});
}

TEST(MatchTest, MatchesMoreThanOneCandidate)
{
    InputManager inputManager;

    ast::StatementsBuilder programBuilder;
    ast::StatementsBuilder statementsBuilder;
    ast::MatchBuilder matchBuilder;

    auto statements = programBuilder
        .addStatement(
            matchBuilder
            .setTarget(ast::makeConstant(Value{String{"1"}}))
            .addCandidatePair(
                    ast::makeConstant(Value{String{"1"}}),
                    statementsBuilder.addStatement(
                        ast::makeAssignment(
                            ast::makeVariable(Name{"x"}),
                            ast::makeConstant(Value{String{"Path 1a taken!"}})
                        )
                    ).build()
            ).addCandidatePair(
                ast::makeConstant(Value{String{"1"}}),
                statementsBuilder.addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"x"}),
                        ast::makeConstant(Value{String{"Path 1b taken!"}})
                    )
                ).build()
            ).build()
        ).build();

    GameInterpreter interpreter(inputManager, Program{std::move(statements)});

    interpreter.execute();

    auto storedX = loadVariable(interpreter, Name{"x"});
    EXPECT_EQ(storedX.asString(), String{"Path 1a taken!"}); // breaks on first match
}
