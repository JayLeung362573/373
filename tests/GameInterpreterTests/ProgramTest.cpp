#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"

#include <stack>


TEST(ProgramTest, LinearNonBlockingProgram)
{
    /**
     * Validates the execution of a linear program that has no blocking statements:
     *
     * x <- 2
     * y <- 5
     * x <- y
     */

    ast::StatementsBuilder programBuilder;

    auto statements = programBuilder
        .addStatement(
            ast::makeAssignment(
                ast::makeVariable(Name{"x"}),
                ast::makeConstant(Value{Integer{2}})
            )
        ).addStatement(
            ast::makeAssignment(
                ast::makeVariable(Name{"y"}),
                ast::makeConstant(Value{Integer{5}})
            )
        ).addStatement(
            ast::makeAssignment(
                ast::makeVariable(Name{"x"}),
                ast::makeVariable(Name{"y"})
            )
        ).build();

    InputManager inputManager;
    GameInterpreter interpreter(inputManager, Program{std::move(statements)});

    interpreter.execute();
    EXPECT_EQ(interpreter.needsIO(), false);

    EXPECT_EQ(
        loadVariable(interpreter, Name{"x"}).asInteger(),
        Integer{5}
    );
    EXPECT_EQ(
        loadVariable(interpreter, Name{"x"}).asInteger(),
        loadVariable(interpreter, Name{"y"}).asInteger()
    );
}

TEST(ProgramTest, LinearBlockingProgram)
{
    /**
     * Validates the execution of a linear program that has a blocking statement:
     *
     * player <- { "id": "100" }
     *
     * input text to player {
     *   prompt: "Enter your answer: "
     *   target: answer
     * }
     *
     * x <- 5
     */

    ast::StatementsBuilder programBuilder;

    Map<String, Value> playerMap{};
    playerMap.setAttribute(String{"id"}, Value{String{"100"}});

    auto statements = programBuilder
        .addStatement(
            ast::makeAssignment(
                ast::makeVariable(Name{"player"}),
                ast::makeConstant(Value{playerMap})
            )
        ).addStatement(
            // Blocking statement!
            ast::makeInputText(
                ast::makeVariable(Name{"player"}),
                ast::makeVariable(Name{"answer"}),
                String{"Enter your answer: "}
            )
        ).addStatement(
            ast::makeAssignment(
                ast::makeVariable(Name{"x"}),
                ast::makeConstant(Value{Integer{5}})
            )
        ).build();

    InputManager inputManager;
    GameInterpreter interpreter(inputManager, Program{std::move(statements)});

    interpreter.execute();
    EXPECT_EQ(interpreter.needsIO(), true);
    EXPECT_EQ(inputManager.getPendingRequests().size(), 1);

    EXPECT_THROW({
        loadVariable(interpreter, Name{"x"});
    }, std::runtime_error);

    inputManager.handleIncomingMessages(
        {GameMessage{
            TextInputMessage{
                String{"100"},
                String{"Enter your answer: "},
                String{"piano"}
            }
        }}
    );
    inputManager.clearPendingRequests();

    interpreter.execute();
    EXPECT_EQ(interpreter.needsIO(), false);
    EXPECT_EQ(inputManager.getPendingRequests().size(), 0);

    EXPECT_EQ(
        loadVariable(interpreter, Name{"answer"}).asString(),
        String{"piano"}
    );
    EXPECT_EQ(
        loadVariable(interpreter, Name{"x"}).asInteger(),
        Integer{5}
    );
}

TEST(ProgramTest, NestedNonBlockingProgram)
{
    /**
     * Validates the execution of a nested program that has no blocking statements:
     *
     * match "1" {
     *   "1" => {
     *     w <- 1
     *
     *     match "1" {
     *       "1" => {
     *         x <- 2
     *       }
     *     }
     *
     *     y <- 3
     *   }
     * }
     *
     * z <- 4
     */

    ast::StatementsBuilder programBuilder;
    ast::MatchBuilder matchBuilder;
    ast::MatchBuilder nestedMatchBuilder;
    ast::StatementsBuilder statementsBuilder;
    ast::StatementsBuilder nestedStatementsBuilder;

    auto statements = programBuilder
        .addStatement(
            matchBuilder
            .setTarget(ast::makeConstant(Value{String{"1"}}))
            .addCandidatePair(
                ast::makeConstant(Value{String{"1"}}),
                statementsBuilder.addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"w"}),
                        ast::makeConstant(Value{Integer{1}})
                    )
                ).addStatement(
                    nestedMatchBuilder
                    .setTarget(ast::makeConstant(Value{String{"1"}}))
                    .addCandidatePair(
                        ast::makeConstant(Value{String{"1"}}),
                        nestedStatementsBuilder
                        .addStatement(
                            ast::makeAssignment(
                                ast::makeVariable(Name{"x"}),
                                ast::makeConstant(Value{Integer{2}})
                            )
                        ).build()
                    ).build()
                ).addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"y"}),
                        ast::makeConstant(Value{Integer{3}})
                    )
                ).build()
            ).build()
        ).addStatement(
            ast::makeAssignment(
                ast::makeVariable(Name{"z"}),
                ast::makeConstant(Value{Integer{4}})
            )
        ).build();

    InputManager inputManager;
    GameInterpreter interpreter(inputManager, Program{std::move(statements)});

    interpreter.execute();
    EXPECT_EQ(interpreter.needsIO(), false);

    EXPECT_EQ(
        loadVariable(interpreter, Name{"w"}).asInteger(),
        Integer{1}
    );
    EXPECT_EQ(
        loadVariable(interpreter, Name{"x"}).asInteger(),
        Integer{2}
    );
    EXPECT_EQ(
        loadVariable(interpreter, Name{"y"}).asInteger(),
        Integer{3}
    );
    EXPECT_EQ(
        loadVariable(interpreter, Name{"z"}).asInteger(),
        Integer{4}
    );
}

TEST(ProgramTest, NestedBlockingProgram)
{
    /**
     * Validates the execution of a nested program that has some blocking statements:
     *
     * match "1" {
     *   "1" => {
     *     player <- { "id": "100" }
     *
     *     match "1" {
     *       "1" => {
     *         x <- 1
     *
     *         input text to player {
     *           prompt: "Enter your answer (1): "
     *           target: answer1
     *         }
     *       }
     *     }
     *
     *     input text to player {
     *       prompt: "Enter your answer (2): "
     *       target: answer2
     *     }
     *
     *     y <- 2
     *   }
     * }
     *
     * z <- 3
     */

    ast::StatementsBuilder programBuilder;
    ast::MatchBuilder matchBuilder;
    ast::MatchBuilder nestedMatchBuilder;
    ast::StatementsBuilder statementsBuilder;
    ast::StatementsBuilder nestedStatementsBuilder;

    Map<String, Value> playerMap{};
    playerMap.setAttribute(String{"id"}, Value{String{"100"}});

    auto statements = programBuilder
        .addStatement(
            matchBuilder
            .setTarget(ast::makeConstant(Value{String{"1"}}))
            .addCandidatePair(
                ast::makeConstant(Value{String{"1"}}),
                statementsBuilder.addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"player"}),
                        ast::makeConstant(Value{playerMap})
                    )
                ).addStatement(
                    nestedMatchBuilder
                    .setTarget(ast::makeConstant(Value{String{"1"}}))
                    .addCandidatePair(
                        ast::makeConstant(Value{String{"1"}}),
                        nestedStatementsBuilder
                        .addStatement(
                            ast::makeAssignment(
                                ast::makeVariable(Name{"x"}),
                                ast::makeConstant(Value{Integer{1}})
                            )
                        // Blocking statement!
                        ).addStatement(
                            ast::makeInputText(
                                ast::makeVariable(Name{"player"}),
                                ast::makeVariable(Name{"answer1"}),
                                String{"Enter your answer (1): "}
                            )
                        ).build()
                    ).build()
                ).addStatement(
                    // Blocking statement!
                    ast::makeInputText(
                        ast::makeVariable(Name{"player"}),
                        ast::makeVariable(Name{"answer2"}),
                        String{"Enter your answer (2): "}
                    )
                ).addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"y"}),
                        ast::makeConstant(Value{Integer{2}})
                    )
                ).build()
            ).build()
        ).addStatement(
            ast::makeAssignment(
                ast::makeVariable(Name{"z"}),
                ast::makeConstant(Value{Integer{3}})
            )
        ).build();

    InputManager inputManager;
    GameInterpreter interpreter(inputManager, Program{std::move(statements)});

    interpreter.execute();
    EXPECT_EQ(interpreter.needsIO(), true);
    EXPECT_EQ(inputManager.getPendingRequests().size(), 1);

    EXPECT_EQ(
        loadVariable(interpreter, Name{"x"}).asInteger(),
        Integer{1}
    );

    EXPECT_THROW({
        loadVariable(interpreter, Name{"y"});
    }, std::runtime_error);

    EXPECT_THROW({
        loadVariable(interpreter, Name{"z"});
    }, std::runtime_error);


    inputManager.handleIncomingMessages(
        {GameMessage{
            TextInputMessage{
                String{"100"},
                String{"Enter your answer (1): "},
                String{"piano"}
            }
        }}
    );
    inputManager.clearPendingRequests();

    interpreter.execute();
    EXPECT_EQ(interpreter.needsIO(), true);
    EXPECT_EQ(inputManager.getPendingRequests().size(), 1);

    EXPECT_EQ(
        loadVariable(interpreter, Name{"answer1"}).asString(),
        String{"piano"}
    );
    EXPECT_THROW({
        loadVariable(interpreter, Name{"y"});
    }, std::runtime_error);

    EXPECT_THROW({
        loadVariable(interpreter, Name{"z"});
    }, std::runtime_error);


    inputManager.handleIncomingMessages(
        {GameMessage{
            TextInputMessage{
                String{"100"},
                String{"Enter your answer (2): "},
                String{"tuba"}
            }
        }}
    );
    inputManager.clearPendingRequests();

    interpreter.execute();
    EXPECT_EQ(interpreter.needsIO(), false);
    EXPECT_EQ(inputManager.getPendingRequests().size(), 0);

    EXPECT_EQ(
        loadVariable(interpreter, Name{"answer2"}).asString(),
        String{"tuba"}
    );
    EXPECT_EQ(
        loadVariable(interpreter, Name{"y"}).asInteger(),
        Integer{2}
    );
    EXPECT_EQ(
        loadVariable(interpreter, Name{"z"}).asInteger(),
        Integer{3}
    );
}

TEST(ProgramTest, ExecuteWhenNoProgram)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    EXPECT_THROW({
        interpreter.execute();
    }, std::runtime_error);
}
