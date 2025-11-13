#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(ShuffleTest, ShuffleList)
{
    // Shuffle is a bit harder to test since the elements are randomly shuffled.
    // But a good enough check might be that two calls to shuffle create different
    // lists. Hopefully 10 distinct elements is enough to not produce the same list
    // twice.
    GameInterpreter interpreter;
    List<Value> list{
        Value{String{"a"}},
        Value{String{"b"}},
        Value{String{"c"}},
        Value{String{"d"}},
        Value{String{"e"}},
        Value{String{"f"}},
        Value{String{"g"}},
        Value{String{"h"}},
        Value{String{"i"}},
        Value{String{"j"}}
    };

    auto listAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"myList"}),
        ast::makeConstant(Value{list})
    );
    doAssignment(interpreter, std::move(listAssignment));

    auto shuffle1 = ast::makeShuffle(ast::makeVariable(Name{"myList"}));
    doShuffle(interpreter, std::move(shuffle1));

    auto shuffle1List = loadVariable(interpreter, Name{"myList"});

    auto shuffle2 = ast::makeShuffle(ast::makeVariable(Name{"myList"}));
    doShuffle(interpreter, std::move(shuffle2));

    auto shuffle2List = loadVariable(interpreter, Name{"myList"});

    EXPECT_EQ(shuffle1List.asList().value.size(), list.value.size());
    EXPECT_EQ(shuffle2List.asList().value.size(), list.value.size());
    EXPECT_NE(shuffle1List, shuffle2List);
}

TEST(ShuffleTest, ShuffleTargetNotAList)
{
    GameInterpreter interpreter;

    auto stringAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"notAList"}),
        ast::makeConstant(Value{String{"a"}})
    );
    doAssignment(interpreter, std::move(stringAssignment));

    auto shuffle = ast::makeShuffle(ast::makeVariable(Name{"notAList"}));

    EXPECT_THROW({
        doShuffle(interpreter, std::move(shuffle));
    }, std::runtime_error);
}
