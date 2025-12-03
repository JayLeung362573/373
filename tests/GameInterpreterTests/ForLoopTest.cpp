#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(ForLoopTest, SimpleForLoop)
{
    InputManager inputManager;

    ast::StatementsBuilder programBuilder;
    ast::StatementsBuilder statementsBuilder;

    List<Value> listOfInts{Value{Integer{10}}, Value{Integer{20}}, Value{Integer{20}}};

    auto statements = programBuilder
        .addStatement(
            ast::makeAssignment(
                ast::makeVariable(Name{"sum"}),
                ast::makeConstant(Value{Integer{0}})
            )
        )
        .addStatement(
            ast::makeForLoop(
                ast::makeVariable(Name{"int"}),
                ast::makeConstant(Value{listOfInts}),
                statementsBuilder.addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"sum"}),
                        ast::makeArithmeticOperation(
                            ast::makeVariable(Name{"sum"}),
                            ast::makeVariable(Name{"int"}),
                            ast::ArithmeticOperation::Kind::ADD
                        )
                    )
                ).build()
            )
        ).build();

    GameInterpreter interpreter(inputManager, Program{std::move(statements)});

    interpreter.execute();

    EXPECT_EQ(
        loadVariable(interpreter, Name{"sum"}).asInteger(), Integer{50}
    );

    EXPECT_THROW({
        loadVariable(interpreter, Name{"int"});
    }, std::runtime_error);
}


TEST(ForLoopTest, NestedForLoop)
{
    InputManager inputManager;

    ast::StatementsBuilder programBuilder;
    ast::StatementsBuilder statementsBuilder1;
    ast::StatementsBuilder statementsBuilder2;

    List<Value> listOfInts{Value{Integer{10}}, Value{Integer{10}}};
    List<Value> nestedListOfInts{Value{Integer{2}}};

    auto statements = programBuilder
        .addStatement(
            ast::makeAssignment(
                ast::makeVariable(Name{"sum"}),
                ast::makeConstant(Value{Integer{0}})
            )
        )
        .addStatement(
            ast::makeForLoop(
                ast::makeVariable(Name{"int"}),
                ast::makeConstant(Value{listOfInts}),
                statementsBuilder1.addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"sum"}),
                        ast::makeArithmeticOperation(
                            ast::makeVariable(Name{"sum"}),
                            ast::makeVariable(Name{"int"}),
                            ast::ArithmeticOperation::Kind::ADD
                        )
                    )
                ).addStatement(
                    ast::makeForLoop(
                        ast::makeVariable(Name{"int"}), // note: same variable name, is ok/allowed
                        ast::makeConstant(Value{nestedListOfInts}),
                        statementsBuilder2.addStatement(
                            ast::makeAssignment(
                                ast::makeVariable(Name{"sum"}),
                                ast::makeArithmeticOperation(
                                    ast::makeVariable(Name{"sum"}),
                                    ast::makeVariable(Name{"int"}),
                                    ast::ArithmeticOperation::Kind::ADD
                                )
                            )
                        ).build()
                    )
                ).build()
            )
        ).build();

    GameInterpreter interpreter(inputManager, Program{std::move(statements)});

    interpreter.execute();

    EXPECT_EQ(
        loadVariable(interpreter, Name{"sum"}).asInteger(), Integer{24}
    );
}


TEST(ForLoopTest, ForLoopWithEmptyList)
{
    InputManager inputManager;

    ast::StatementsBuilder programBuilder;
    ast::StatementsBuilder statementsBuilder;

    auto statements = programBuilder
        .addStatement(
            ast::makeAssignment(
                ast::makeVariable(Name{"flag"}),
                ast::makeConstant(Value{Boolean{false}})
            )
        )
        .addStatement(
            ast::makeForLoop(
                ast::makeVariable(Name{"int"}),
                ast::makeConstant(Value{List<Value>()}),
                statementsBuilder.addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"flag"}),
                        ast::makeConstant(Value{Boolean{true}})
                    )
                ).build()
            )
        ).build();

    GameInterpreter interpreter(inputManager, Program{std::move(statements)});

    interpreter.execute();

    EXPECT_EQ(
        loadVariable(interpreter, Name{"flag"}).asBoolean(), Boolean{false}
    );
}


TEST(ForLoopTest, ForLoopWithIO)
{
    InputManager inputManager;

    ast::StatementsBuilder programBuilder;
    ast::StatementsBuilder statementsBuilder;

    Map<String, Value> player{};
    player.setAttribute(String{"id"}, Value{String{"1"}});

    auto statements = programBuilder
        .addStatement(
            ast::makeAssignment(
                ast::makeVariable(Name{"player"}),
                ast::makeConstant(Value{player})
            )
        )
        .addStatement(
            ast::makeAssignment(
                ast::makeVariable(Name{"sum"}),
                ast::makeConstant(Value{Integer{0}})
            )
        )
        .addStatement(
            ast::makeForLoop(
                ast::makeVariable(Name{"_"}),
                ast::makeConstant(Value{List{Value{Integer{0}}, Value{Integer{1}}}}),
                statementsBuilder.addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"sum"}),
                        ast::makeArithmeticOperation(
                            ast::makeVariable(Name{"sum"}),
                            ast::makeConstant(Value{Integer{1}}),
                            ast::ArithmeticOperation::Kind::ADD
                        )
                    )
                // Blocking statement!
                ).addStatement(
                    ast::makeInputText(
                        ast::makeVariable(Name{"player"}),
                        ast::makeVariable(Name{"answer"}),
                        String{"Enter your answer: "}
                    )
                ).addStatement(
                    ast::makeAssignment(
                        ast::makeVariable(Name{"sum"}),
                        ast::makeArithmeticOperation(
                            ast::makeVariable(Name{"sum"}),
                            ast::makeConstant(Value{Integer{1}}),
                            ast::ArithmeticOperation::Kind::ADD
                        )
                    )
                ).build()
            )
        ).build();

    GameInterpreter interpreter(inputManager, Program{std::move(statements)});

    interpreter.execute();
    EXPECT_EQ(interpreter.needsIO(), true);

    EXPECT_EQ(
        loadVariable(interpreter, Name{"sum"}).asInteger(), Integer{1}
    );

    EXPECT_THROW({
        loadVariable(interpreter, Name{"answer"});
    }, std::runtime_error);

    inputManager.handleIncomingMessages(
        {GameMessage{
            TextInputMessage{
                String{"1"},
                String{"Enter your answer: "},
                String{"cat"}
            }
        }}
    );
    inputManager.clearPendingRequests();

    interpreter.execute();
    EXPECT_EQ(interpreter.needsIO(), true);

    EXPECT_EQ(
        loadVariable(interpreter, Name{"sum"}).asInteger(), Integer{3}
    );

    EXPECT_EQ(
        loadVariable(interpreter, Name{"answer"}).asString(), String{"cat"}
    );

    inputManager.handleIncomingMessages(
        {GameMessage{
            TextInputMessage{
                String{"1"},
                String{"Enter your answer: "},
                String{"dog"}
            }
        }}
    );
    inputManager.clearPendingRequests();

    interpreter.execute();
    EXPECT_EQ(interpreter.needsIO(), false);

    EXPECT_EQ(
        loadVariable(interpreter, Name{"sum"}).asInteger(), Integer{4}
    );

    EXPECT_EQ(
        loadVariable(interpreter, Name{"answer"}).asString(), String{"dog"}
    );
}
