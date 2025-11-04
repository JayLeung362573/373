#include <gtest/gtest.h>
#include <optional>
#include <iostream>
#include "GameInterpreter.h"

// test helpers
void
doAssignment(GameInterpreter& interpreter, std::unique_ptr<ast::Assignment> assignment)
{
    VisitResult result = assignment->accept(interpreter);
    EXPECT_TRUE(result.isDone());
    EXPECT_FALSE(result.hasValue());
    EXPECT_FALSE(result.isReference());
}

Value
loadVariable(GameInterpreter& interpreter, Name targetName)
{
    ast::Variable loadVariable(targetName);
    VisitResult result = loadVariable.accept(interpreter);

    EXPECT_TRUE(result.isDone());
    EXPECT_TRUE(result.hasValue());
    EXPECT_TRUE(result.isReference());

    return result.getValue();
}

TEST(GameInterpreterTest, VisitConstant)
{
    GameInterpreter interpreter;
    Value value{String{"100"}};
    auto constant = ast::makeConstant(value);

    VisitResult result = constant->accept(interpreter);

    EXPECT_EQ(result.status, VisitResult::Status::Done);
    EXPECT_TRUE(result.isDone());
    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(result.getValue(), value);
    EXPECT_FALSE(result.isReference());
}

TEST(GameInterpreterTest, AssignStringToVariable)
{
    GameInterpreter interpreter;

    auto assignment = ast::makeAssignment(
        ast::makeVariable(Name{"score"}),
        ast::makeConstant(Value{String{"100"}})
    );
    doAssignment(interpreter, std::move(assignment));

    Value storedVar = loadVariable(interpreter, Name{"score"});
    EXPECT_EQ(storedVar, Value{String{"100"}});
}

TEST(GameInterpreterTest, AssignMapToVariable)
{
    GameInterpreter interpreter;

    Map<String, Value> map{};
    map.setAttribute(String{"a"}, Value{String{"1"}});

    auto varAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"myMap"}),
        ast::makeConstant(Value{map})
    );
    doAssignment(interpreter, std::move(varAssignment));

    Value storedVar = loadVariable(interpreter, Name{"myMap"});
    EXPECT_EQ(storedVar.getAttribute(String{"a"}), Value{String{"1"}});
}

TEST(GameInterpreterTest, AssignVariableMapAttribute)
{
    GameInterpreter interpreter;

    Map<String, Value> map{};

    auto varAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"myMap"}),
        ast::makeConstant(Value{map})
    );
    doAssignment(interpreter, std::move(varAssignment));

    Value storedVar = loadVariable(interpreter, Name{"myMap"});
    EXPECT_EQ(storedVar.asMap().value.size(), 0);

    auto attrAssignment  = ast::makeAssignment(
        ast::makeAttribute(
            ast::makeVariable(
                Name{"myMap"}
            ),
            String{"a"}
        ),
        ast::makeConstant(Value{String{"1"}})
    );
    doAssignment(interpreter, std::move(attrAssignment));

    storedVar = loadVariable(interpreter, Name{"myMap"});
    EXPECT_EQ(storedVar.asMap().value.size(), 1);
    EXPECT_EQ(storedVar.getAttribute(String{"a"}), Value{String{"1"}});
}

TEST(GameInterpreterTest, AssignVariableNestedMapAttribute)
{
    GameInterpreter interpreter;

    Map<String, Value> map{};
    Map<String, Value> nestedMap{};
    nestedMap.setAttribute(String{"a"}, Value{String{"1"}});
    map.setAttribute(String{"myNestedMap"}, Value{nestedMap});

    auto varAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"myMap"}),
        ast::makeConstant(Value{map})
    );
    doAssignment(interpreter, std::move(varAssignment));

    auto loadedMap = loadVariable(interpreter, Name{"myMap"}).asMap();
    auto loadedNestedMap = loadedMap.getAttribute(String{"myNestedMap"}).asMap();
    EXPECT_EQ(loadedNestedMap.getAttribute(String{"a"}), Value{String{"1"}});

    auto nestedAttrAssignment = ast::makeAssignment(
        ast::makeAttribute(
            ast::makeAttribute(
                ast::makeVariable(Name{"myMap"}),
                String{"myNestedMap"}
            ),
            String{"a"}
        ),
        ast::makeConstant(Value{String{"2"}})
    );
    doAssignment(interpreter, std::move(nestedAttrAssignment));

    loadedMap = loadVariable(interpreter, Name{"myMap"}).asMap();
    loadedNestedMap = loadedMap.getAttribute(String{"myNestedMap"}).asMap();
    EXPECT_EQ(loadedNestedMap.getAttribute(String{"a"}), Value{String{"2"}});
}

TEST(GameInterpreterTest, AssignVariableToVariableString)
{
    GameInterpreter interpreter;

    auto var1Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var1"}),
        ast::makeConstant(Value{String{"a"}})
    );
    auto var2Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var2"}),
        ast::makeConstant(Value{String{"b"}})
    );
    doAssignment(interpreter, std::move(var1Assignment));
    doAssignment(interpreter, std::move(var2Assignment));

    Value storedVar1 = loadVariable(interpreter, Name{"var1"});
    Value storedVar2 = loadVariable(interpreter, Name{"var2"});
    EXPECT_EQ(storedVar1, Value{String{"a"}});
    EXPECT_EQ(storedVar2, Value{String{"b"}});

    auto var1ToVar2Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var1"}),
        ast::makeVariable(Name{"var2"})
    );
    doAssignment(interpreter, std::move(var1ToVar2Assignment));

    storedVar1 = loadVariable(interpreter, Name{"var1"});
    storedVar2 = loadVariable(interpreter, Name{"var2"});
    EXPECT_EQ(storedVar1, Value{String{"b"}});
    EXPECT_EQ(storedVar2, Value{String{"b"}});

    // Check - does var1 hold a copy or reference of var2's value?
    auto var2Modify = ast::makeAssignment(
        ast::makeVariable(Name{"var2"}),
        ast::makeConstant(Value{String{"c"}})
    );
    doAssignment(interpreter, std::move(var2Modify));

    // var1 should not reflect the changes made to var2 if it was a copy
    storedVar1 = loadVariable(interpreter, Name{"var1"});
    storedVar2 = loadVariable(interpreter, Name{"var2"});
    EXPECT_EQ(storedVar1, Value{String{"b"}});
    EXPECT_EQ(storedVar2, Value{String{"c"}});
}

TEST(GameInterpreterTest, AssignVariableToVariableMap)
{
    GameInterpreter interpreter;

    Map<String, Value> map;
    map.setAttribute(String{"a"}, Value{String{"1"}});

    auto var2Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var2"}),
        ast::makeConstant(Value{map})
    );
    doAssignment(interpreter, std::move(var2Assignment));

    Value storedVar1;
    Value storedVar2 = loadVariable(interpreter, Name{"var2"});
    EXPECT_EQ(storedVar2.getAttribute(String{"a"}), Value{String{"1"}});

    auto var1ToVar2Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var1"}),
        ast::makeVariable(Name{"var2"})
    );
    doAssignment(interpreter, std::move(var1ToVar2Assignment));

    storedVar1 = loadVariable(interpreter, Name{"var1"});
    storedVar2 = loadVariable(interpreter, Name{"var2"});
    EXPECT_EQ(storedVar1.getAttribute(String{"a"}), Value{String{"1"}});
    EXPECT_EQ(storedVar2.getAttribute(String{"a"}), Value{String{"1"}});

    // Check - does var1 hold a copy or reference of var2's value?
    auto var2Modify = ast::makeAssignment(
        ast::makeAttribute(
            ast::makeVariable(Name{"var2"}),
            String{"a"}
        ),
        ast::makeConstant(Value{String{"2"}})
    );
    doAssignment(interpreter, std::move(var2Modify));

    // var1 should not reflect the changes made to var2 if it was a copy
    storedVar1 = loadVariable(interpreter, Name{"var1"});
    storedVar2 = loadVariable(interpreter, Name{"var2"});
    EXPECT_EQ(storedVar1.getAttribute(String{"a"}), Value{String{"1"}});
    EXPECT_EQ(storedVar2.getAttribute(String{"a"}), Value{String{"2"}});
}

TEST(GameInterpreterTest, AssignConstant)
{
    GameInterpreter interpreter;
    auto invalidAssignment = ast::makeAssignment(
        ast::makeConstant(Value{String{"a"}}), // Invalid syntax, constants are not assignable
        ast::makeConstant(Value{String{"b"}})
    );

    EXPECT_THROW({
        doAssignment(interpreter, std::move(invalidAssignment));
    }, std::runtime_error);
}

TEST(GameInterpreterTest, AssignUndefinedVariable)
{
    GameInterpreter interpreter;
    auto invalidAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"var1"}),
        ast::makeVariable(Name{"var2"}) // var2 is undefined
    );

    EXPECT_THROW({
        doAssignment(interpreter, std::move(invalidAssignment));
    }, std::runtime_error);
}

TEST(GameInterpreterTest, AssignUndefinedAttribute)
{
    GameInterpreter interpreter;
    Map<String, Value> map;
    map.setAttribute(String{"a"}, Value{String{"1"}});

    auto var1Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var1"}),
        ast::makeConstant(Value{map})
    );
    doAssignment(interpreter, std::move(var1Assignment));

    auto invalidVar2Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var2"}),
        ast::makeAttribute(
            ast::makeVariable(Name{"var1"}),
            String{"b"} // var1 doesn't have a "b" attribute
        )
    );

    EXPECT_THROW({
        doAssignment(interpreter, std::move(invalidVar2Assignment));
    }, std::runtime_error);
}

TEST(GameInterpreterTest, InputTextStatementOutputsMessageToGetInput)
{
    GameInterpreter interpreter;

    Map<String, Value> playerMap{};
    Name playerMapName{"player"};
    Name targetName{"answer"};
    playerMap.setAttribute(String{"id"}, Value{String{"123"}});

    auto playerMapAssignment = ast::makeAssignment(
        ast::makeVariable(playerMapName),
        ast::makeConstant(Value{playerMap})
    );
    doAssignment(interpreter, std::move(playerMapAssignment));

    auto inputTextStmt = ast::makeInputTextStmt(
        ast::makeVariable(playerMapName),
        ast::makeVariable(targetName),
        String{"Enter your answer: "}
    );

    VisitResult result = inputTextStmt->accept(interpreter);
    EXPECT_TRUE(result.isPending());
    EXPECT_FALSE(result.hasValue());

    std::vector<GameMessage> outMessages = interpreter.consumeOutGameMessages();
    ASSERT_EQ(outMessages.size(), 1);

    auto& msg = outMessages[0].inner;
    ASSERT_TRUE(std::holds_alternative<GetTextInputMessage>(msg));
    GetTextInputMessage getInputMsg = std::get<GetTextInputMessage>(msg);

    EXPECT_EQ(getInputMsg.playerID, String{"123"});
    EXPECT_EQ(getInputMsg.prompt, String{"Enter your answer: "});
}

TEST(GameInterpreterTest, InputTextStatementHandlesInput)
{
    GameInterpreter interpreter;

    Map<String, Value> playerMap{};
    Name playerMapName{"player"};
    Name targetName{"answer"};
    playerMap.setAttribute(String{"id"}, Value{String{"123"}});

    auto playerMapAssignment = ast::makeAssignment(
        ast::makeVariable(playerMapName),
        ast::makeConstant(Value{playerMap})
    );
    doAssignment(interpreter, std::move(playerMapAssignment));

    auto inputTextStmt = ast::makeInputTextStmt(
        ast::makeVariable(playerMapName),
        ast::makeVariable(targetName),
        String{"Enter your answer: "}
    );

    TextInputMessage giveInputMsg{
        String{"123"},
        String{"Enter your answer: "},
        {"piano"}
    };

    std::vector<GameMessage> inMessages{GameMessage{giveInputMsg}};
    interpreter.setInGameMessages(inMessages);

    VisitResult result = inputTextStmt->accept(interpreter);
    EXPECT_TRUE(result.isDone());
    EXPECT_FALSE(result.hasValue());

    std::vector<GameMessage> outMessages = interpreter.consumeOutGameMessages();
    ASSERT_EQ(outMessages.size(), 0);

    Value storedAnswer = loadVariable(interpreter, targetName);
    EXPECT_EQ(storedAnswer.asString(), String{"piano"});
}
