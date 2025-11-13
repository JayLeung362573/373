#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(ConstantTest, VisitConstant)
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
