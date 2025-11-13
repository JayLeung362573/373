#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"

TEST(ReverseTest, ReverseList)
{
    GameInterpreter interpreter;
    List<Value> list{Value{String{"a"}}, Value{String{"b"}}, Value{String{"c"}}};
    List<Value> expectedReversedList{
        Value{String{"c"}},
        Value{String{"b"}},
        Value{String{"a"}}
    };

    auto listAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"myList"}),
        ast::makeConstant(Value{list})
    );
    doAssignment(interpreter, std::move(listAssignment));

    auto reverse = ast::makeReverse(ast::makeVariable(Name{"myList"}));
    doReverse(interpreter, std::move(reverse));

    auto storedList = loadVariable(interpreter, Name{"myList"});
    EXPECT_EQ(storedList.asList(), expectedReversedList);
}

TEST(ReverseTest, ReverseTargetNotAList)
{
    GameInterpreter interpreter;

    auto stringAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"notAList"}),
        ast::makeConstant(Value{String{"a"}})
    );
    doAssignment(interpreter, std::move(stringAssignment));

    auto reverse = ast::makeReverse(ast::makeVariable(Name{"notAList"}));

    EXPECT_THROW({
        doReverse(interpreter, std::move(reverse));
    }, std::runtime_error);
}
