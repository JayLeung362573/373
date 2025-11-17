#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(MatchTest, MatchWithMultipleCandidates)
{
    GameInterpreter interpreter;

    ast::StatementsBuilder statementsBuilder;
    ast::MatchBuilder matchBuilder;

    auto match = matchBuilder
                .setTarget(ast::makeConstant(Value{String{"2"}}))
                .addCandidatePair(
                    {
                        ast::makeConstant(Value{String{"1"}}),
                        statementsBuilder.addStatement(
                            ast::makeAssignment(
                                ast::makeVariable(Name{"x"}),
                                ast::makeConstant(Value{String{"Path 1 taken!"}})
                            )
                        ).build()
                    }
                )
                .addCandidatePair(
                    {
                        ast::makeConstant(Value{String{"2"}}),
                        statementsBuilder.addStatement(
                            ast::makeAssignment(
                                ast::makeVariable(Name{"x"}),
                                ast::makeConstant(Value{String{"Path 2 taken!"}})
                            )
                        ).build()
                    }
                )
                .build();

    doMatch(interpreter, std::move(match));

    auto storedX = loadVariable(interpreter, Name{"x"});
    EXPECT_EQ(storedX.asString(), String{"Path 2 taken!"});
}

TEST(MatchTest, NoMatch)
{
    GameInterpreter interpreter;

    ast::StatementsBuilder statementsBuilder;
    ast::MatchBuilder matchBuilder;

    auto match = matchBuilder
                .setTarget(ast::makeConstant(Value{String{"2"}}))
                .addCandidatePair(
                    {
                        ast::makeConstant(Value{String{"1"}}),
                        statementsBuilder.addStatement(
                            ast::makeAssignment(
                                ast::makeVariable(Name{"x"}),
                                ast::makeConstant(Value{String{"Path 1 taken!"}})
                            )
                        ).build()
                    }
                )
                .build();

    doMatch(interpreter, std::move(match));

    EXPECT_THROW({
        // x shouldn't be defined because assignment statement shouldn't have executed
        loadVariable(interpreter, Name{"x"});
    }, std::runtime_error);
}


TEST(MatchTest, MultipleStatements)
{
    GameInterpreter interpreter;

    ast::StatementsBuilder statementsBuilder;
    ast::MatchBuilder matchBuilder;

    auto match = matchBuilder
                .setTarget(ast::makeConstant(Value{String{"1"}}))
                .addCandidatePair(
                    {
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
                    }
                )
                .build();

    doMatch(interpreter, std::move(match));

    auto storedX = loadVariable(interpreter, Name{"x"});
    auto storedY = loadVariable(interpreter, Name{"y"});
    EXPECT_EQ(storedX.asString(), String{"Path 1 taken!"});
    EXPECT_EQ(storedY.asString(), String{"Path 1 taken!"});
}

TEST(MatchTest, MatchesMoreThanOneCandidate)
{
    GameInterpreter interpreter;

    ast::StatementsBuilder statementsBuilder;
    ast::MatchBuilder matchBuilder;

    auto match = matchBuilder
                .setTarget(ast::makeConstant(Value{String{"1"}}))
                .addCandidatePair(
                    {
                        ast::makeConstant(Value{String{"1"}}),
                        statementsBuilder.addStatement(
                            ast::makeAssignment(
                                ast::makeVariable(Name{"x"}),
                                ast::makeConstant(Value{String{"Path 1a taken!"}})
                            )
                        ).build()
                    }
                )
                .addCandidatePair(
                    {
                        ast::makeConstant(Value{String{"1"}}),
                        statementsBuilder.addStatement(
                            ast::makeAssignment(
                                ast::makeVariable(Name{"x"}),
                                ast::makeConstant(Value{String{"Path 1b taken!"}})
                            )
                        ).build()
                    }
                )
                .build();

    doMatch(interpreter, std::move(match));

    auto storedX = loadVariable(interpreter, Name{"x"});
    EXPECT_EQ(storedX.asString(), String{"Path 1a taken!"}); // breaks on first match
}
