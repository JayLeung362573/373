#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(ExtendTest, ExtendList)
{
    GameInterpreter interpreter;
    List<Value> list1{Value{String{"a"}}, Value{String{"b"}}};
    List<Value> list2{Value{String{"c"}}};
    List<Value> expectedExtendedList{
        Value{String{"a"}},
        Value{String{"b"}},
        Value{String{"c"}}
    };

    auto listAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"list1"}),
        ast::makeConstant(Value{list1})
    );
    doAssignment(interpreter, std::move(listAssignment));

    auto extend = ast::makeExtend(
        ast::makeVariable(Name{"list1"}),
        ast::makeConstant(Value{list2})
    );
    doExtend(interpreter, std::move(extend));

    auto storedList1 = loadVariable(interpreter, Name{"list1"});
    EXPECT_EQ(storedList1.asList(), expectedExtendedList);
}

TEST(ExtendTest, ExtendTargetNotAList)
{
    GameInterpreter interpreter;
    List<Value> list{Value{String{"c"}}};

    auto stringAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"notAList"}),
        ast::makeConstant(Value{String{"a"}})
    );
    doAssignment(interpreter, std::move(stringAssignment));

    auto extend = ast::makeExtend(
        ast::makeVariable(Name{"notAList"}), // not a list!
        ast::makeConstant(Value{list})
    );

    EXPECT_THROW({
        doExtend(interpreter, std::move(extend));
    }, std::runtime_error);
}

TEST(ExtendTest, ExtendValueNotAList)
{
    GameInterpreter interpreter;
    List<Value> list{Value{String{"a"}}, Value{String{"b"}}};

    auto listAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"myList"}),
        ast::makeConstant(Value{list})
    );
    doAssignment(interpreter, std::move(listAssignment));

    auto extend = ast::makeExtend(
        ast::makeVariable(Name{"myList"}),
        ast::makeConstant(Value{String{"c"}}) // not a list!
    );

    EXPECT_THROW({
        doExtend(interpreter, std::move(extend));
    }, std::runtime_error);
}
