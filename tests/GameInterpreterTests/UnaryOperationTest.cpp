#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(UnaryOperationTest, NotOnFalse)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    auto unaryOp = ast::makeUnaryOperation(
        // false
        ast::makeComparison(
            ast::makeConstant(Value{String{"a"}}),
            ast::makeConstant(Value{String{"b"}}),
            ast::Comparison::Kind::EQ
        ),
        ast::UnaryOperation::Kind::NOT
        // -> NOT evaluates to true
    );

    VisitResult result = doUnaryOperation(interpreter, std::move(unaryOp));

    EXPECT_EQ(result.getValue(), Value{Boolean{true}});
}

TEST(UnaryOperationTest, NotOnTrue)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    auto unaryOp = ast::makeUnaryOperation(
        // true
        ast::makeConstant(Value{Boolean{true}}),
        ast::UnaryOperation::Kind::NOT
        // -> NOT evaluates to false
    );

    VisitResult result = doUnaryOperation(interpreter, std::move(unaryOp));

    EXPECT_EQ(result.getValue(), Value{Boolean{false}});
}
