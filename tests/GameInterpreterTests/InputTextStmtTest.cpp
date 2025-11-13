#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(InputTextStmtTest, InputTextStatementOutputsMessageToGetInput)
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

TEST(InputTextStmtTest, InputTextStatementHandlesInput)
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
