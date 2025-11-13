#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(ComparisonTest, IsEqual)
{
    GameInterpreter interpreter;

    auto comparison = ast::makeComparison(
        ast::makeConstant(Value{String{"a"}}),
        ast::makeConstant(Value{String{"a"}}),
        ast::Comparison::Kind::EQ
    );

    VisitResult result = doComparison(interpreter, std::move(comparison));

    EXPECT_EQ(result.getValue(), Value{Boolean{true}});
}

TEST(ComparisonTest, IsLessThan)
{
    GameInterpreter interpreter;

    auto comparison = ast::makeComparison(
        ast::makeConstant(Value{Integer{-1}}),
        ast::makeConstant(Value{Integer{1}}),
        ast::Comparison::Kind::LT
    );

    VisitResult result = doComparison(interpreter, std::move(comparison));

    EXPECT_EQ(result.getValue(), Value{Boolean{true}});
}

TEST(ComparisonTest, TypesNotComparable)
{
    GameInterpreter interpreter;

    auto comparison = ast::makeComparison(
        ast::makeConstant(Value{Integer{1}}),
        ast::makeConstant(Value{String{"a"}}),
        ast::Comparison::Kind::LT
    );

    EXPECT_THROW({
        doComparison(interpreter, std::move(comparison));
    }, std::runtime_error);
}

TEST(ComparisonTest, VariableComparisons)
{
    GameInterpreter interpreter;

    auto var1Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var1"}),
        ast::makeConstant(Value{Integer{10}})
    );
    auto var2Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var2"}),
        ast::makeConstant(Value{Integer{10}})
    );
    auto var3Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var3"}),
        ast::makeConstant(Value{Integer{15}})
    );
    doAssignment(interpreter, std::move(var1Assignment));
    doAssignment(interpreter, std::move(var2Assignment));
    doAssignment(interpreter, std::move(var3Assignment));

    auto comparison1 = ast::makeComparison(
        ast::makeVariable(Name{"var1"}),
        ast::makeVariable(Name{"var2"}),
        ast::Comparison::Kind::EQ
    );
    auto comparison2 = ast::makeComparison(
        ast::makeVariable(Name{"var1"}),
        ast::makeVariable(Name{"var3"}),
        ast::Comparison::Kind::EQ
    );

    VisitResult result1 = doComparison(interpreter, std::move(comparison1));
    VisitResult result2 = doComparison(interpreter, std::move(comparison2));
    EXPECT_EQ(result1.getValue(), Value{Boolean{true}});
    EXPECT_EQ(result2.getValue(), Value{Boolean{false}});
}

TEST(ComparisonTest, NestedComparisons)
{
    GameInterpreter interpreter;

    auto comparison = ast::makeComparison(
        ast::makeComparison(
            ast::makeConstant(Value{Integer{100}}),
            ast::makeConstant(Value{Integer{500}}),
            ast::Comparison::Kind::LT
        ),
        ast::makeComparison(
            ast::makeConstant(Value{Integer{200}}),
            ast::makeConstant(Value{Integer{500}}),
            ast::Comparison::Kind::LT
        ),
        ast::Comparison::Kind::EQ
    );

    VisitResult result = doComparison(interpreter, std::move(comparison));
    EXPECT_EQ(result.getValue(), Value{Boolean{true}});
}
