#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(SortTest, SortList)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    List<Value> myList{Value{String{"c"}}, Value{String{"a"}}, Value{String{"b"}}};
    List<Value> expected{Value{String{"a"}}, Value{String{"b"}}, Value{String{"c"}}};

    auto listAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"myList"}),
        ast::makeConstant(Value{myList})
    );
    doAssignment(interpreter, std::move(listAssignment));

    auto sort = ast::makeSort(ast::makeVariable(Name{"myList"}));
    doSort(interpreter, std::move(sort));

    auto storedList = loadVariable(interpreter, Name{"myList"});
    EXPECT_EQ(storedList.asList(), expected);
}

TEST(SortTest, SortListWithKey)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    Map<String, Value> map1;
    Map<String, Value> map2;
    Map<String, Value> map3;
    map1.setAttribute(String{"damage"}, Value{Integer{90}});
    map2.setAttribute(String{"damage"}, Value{Integer{10}});
    map3.setAttribute(String{"damage"}, Value{Integer{30}});

    List<Value> myList{Value{map1}, Value{map2}, Value{map3}};
    List<Value> expected{Value{map2}, Value{map3}, Value{map1}};

    auto listAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"myList"}),
        ast::makeConstant(Value{myList})
    );
    doAssignment(interpreter, std::move(listAssignment));

    auto sort = ast::makeSort(
        ast::makeVariable(Name{"myList"}), String{"damage"}
    );
    doSort(interpreter, std::move(sort));

    auto storedList = loadVariable(interpreter, Name{"myList"});
    EXPECT_EQ(storedList.asList(), expected);
}

TEST(SortTest, SortTargetNotAList)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    auto stringAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"notAList"}),
        ast::makeConstant(Value{String{"a"}})
    );
    doAssignment(interpreter, std::move(stringAssignment));

    auto sort = ast::makeSort(
        ast::makeVariable(Name{"notAList"}) // not a list!
    );

    EXPECT_THROW({
        doSort(interpreter, std::move(sort));
    }, std::runtime_error);
}
