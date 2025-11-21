#include <gtest/gtest.h>
#include "Rules.h"

TEST(RulesTest, MakeConstant)
{
    auto constant = ast::makeConstant(Value{String{"hello"}});
    EXPECT_EQ(constant->getValue(), Value{String{"hello"}});
}

TEST(RulesTest, MakeVariable)
{
    auto variable = ast::makeVariable(Name{"player"});
    EXPECT_EQ(variable->getName(), Name{"player"});
}

TEST(RulesTest, MakeAttribute)
{
    auto attr = ast::makeAttribute(
        ast::makeVariable(Name{"player"}),
        String{"name"}
    );

    EXPECT_NE(attr, nullptr);
    EXPECT_EQ(attr->getAttr(), String{"name"});

    auto baseVar = ast::castExpressionToVariable(attr->getBase());
    EXPECT_NE(baseVar, nullptr);
    EXPECT_EQ(baseVar->getName(), Name{"player"});
}

TEST(RulesTest, MakeAttributeNested)
{
    auto attr = ast::makeAttribute(
        ast::makeAttribute(
            ast::makeVariable(Name{"players"}),
            String{"player_1"}
        ),
        String{"score"}
    );
    EXPECT_NE(attr, nullptr);
    EXPECT_EQ(attr->getAttr(), String{"score"});

    auto baseAttr = castExpressionToAttribute(attr->getBase());
    EXPECT_NE(baseAttr, nullptr);
    EXPECT_EQ(baseAttr->getAttr(), String{"player_1"});

    auto baseVar = castExpressionToVariable(baseAttr->getBase());
    EXPECT_NE(baseVar, nullptr);
    EXPECT_EQ(baseVar->getName(), Name{"players"});
}

TEST(RulesTest, MakeAssignment)
{
    auto assignment = ast::makeAssignment(
        ast::makeVariable(Name{"rounds"}),
        ast::makeConstant(Value{String{"3"}})
    );

    auto targetVar = ast::castExpressionToVariable(assignment->getTarget());
    EXPECT_NE(targetVar, nullptr);
    EXPECT_EQ(targetVar->getName(), Name{"rounds"});

    auto valueConstant = ast::castExpressionToConstant(assignment->getValue());
    ASSERT_NE(valueConstant, nullptr);
    EXPECT_EQ(valueConstant->getValue(), Value{String{"3"}});
}

TEST(RulesTest, MakeInputTextStatementWithVariableTarget)
{
    auto inputTextStmt = ast::makeInputText(
        ast::makeVariable(Name{"player"}),
        ast::makeVariable(Name{"seed"}),
        String{"Enter a seed: "}
    );

    EXPECT_EQ(inputTextStmt->getPlayer()->getName(), Name{"player"});

    auto targetVar = ast::castExpressionToVariable(inputTextStmt->getTarget());
    EXPECT_NE(targetVar, nullptr);
    EXPECT_EQ(targetVar->getName(), Name{"seed"});
    EXPECT_EQ(inputTextStmt->getPrompt(), String{"Enter a seed: "});
}

TEST(RulesTest, MakeInputTextStatementWithAttributeTarget)
{
    auto inputTextStmt = ast::makeInputText(
        ast::makeVariable(Name{"playerMap"}),
        ast::makeAttribute(
            ast::makeVariable(Name{"playerMap"}),
            String{"seed"}
        ),
        String{"Enter a seed: "}
    );

    EXPECT_EQ(inputTextStmt->getPlayer()->getName(), Name{"playerMap"});
    EXPECT_EQ(inputTextStmt->getPrompt(), String{"Enter a seed: "});

    auto targetAttr = castExpressionToAttribute(inputTextStmt->getTarget());
    EXPECT_NE(targetAttr, nullptr);
    EXPECT_EQ(targetAttr->getAttr(), String{"seed"});

    auto baseVar = castExpressionToVariable(targetAttr->getBase());
    EXPECT_NE(baseVar, nullptr);
    EXPECT_EQ(baseVar->getName(), Name{"playerMap"});
}
