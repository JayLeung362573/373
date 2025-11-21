#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(LogicalOperationTest, OrIsTrue)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    auto logicalOp = ast::makeLogicalOperation(
        // false
        ast::makeComparison(
            ast::makeConstant(Value{String{"a"}}),
            ast::makeConstant(Value{String{"b"}}),
            ast::Comparison::Kind::EQ
        ),
        // true
        ast::makeComparison(
            ast::makeConstant(Value{String{"a"}}),
            ast::makeConstant(Value{String{"a"}}),
            ast::Comparison::Kind::EQ
        ),
        ast::LogicalOperation::Kind::OR
        // -> OR evaluates to true
    );

    VisitResult result = doLogicalOperation(interpreter, std::move(logicalOp));

    EXPECT_EQ(result.getValue(), Value{Boolean{true}});
}

TEST(LogicalOperationTest, OrIsFalse)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    auto logicalOp = ast::makeLogicalOperation(
        // false
        ast::makeComparison(
            ast::makeConstant(Value{String{"a"}}),
            ast::makeConstant(Value{String{"b"}}),
            ast::Comparison::Kind::EQ
        ),
        // false
        ast::makeConstant(Value{Boolean{false}}),
        ast::LogicalOperation::Kind::OR
        // -> OR evaluates to false
    );

    VisitResult result = doLogicalOperation(interpreter, std::move(logicalOp));

    EXPECT_EQ(result.getValue(), Value{Boolean{false}});
}
