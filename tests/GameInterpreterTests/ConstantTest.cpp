#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(ConstantTest, VisitConstant)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    Value value{String{"100"}};
    auto constant = ast::makeConstant(value);

    VisitResult result = constant->accept(interpreter);

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(result.getValue(), value);
    EXPECT_FALSE(result.isReference());
}
