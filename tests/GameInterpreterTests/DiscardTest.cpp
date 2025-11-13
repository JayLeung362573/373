#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(DiscardTest, DiscardList)
{
    GameInterpreter interpreter;
    List<Value> myList{Value{String{"a"}}, Value{String{"b"}}};
    List<Value> expectedListAfterDiscard{};

    auto listAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"myList"}),
        ast::makeConstant(Value{myList})
    );
    doAssignment(interpreter, std::move(listAssignment));

    auto discard = ast::makeDiscard(
        ast::makeVariable(Name{"myList"}),
        ast::makeConstant(Value{Integer{2}}) // amount to discard
    );
    doDiscard(interpreter, std::move(discard));

    auto storedList1 = loadVariable(interpreter, Name{"myList"});
    EXPECT_EQ(storedList1.asList(), expectedListAfterDiscard);
}

TEST(DiscardTest, DiscardTargetNotAList)
{
    GameInterpreter interpreter;

    auto stringAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"notAList"}),
        ast::makeConstant(Value{String{"a"}})
    );
    doAssignment(interpreter, std::move(stringAssignment));

    auto discard = ast::makeDiscard(
        ast::makeVariable(Name{"notAList"}), // not a list!
        ast::makeConstant(Value{Integer{2}})
    );

    EXPECT_THROW({
        doDiscard(interpreter, std::move(discard));
    }, std::runtime_error);
}

TEST(DiscardTest, DiscardAmountNotAnInteger)
{
    GameInterpreter interpreter;
    List<Value> myList{Value{String{"a"}}, Value{String{"b"}}};

    auto listAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"myList"}),
        ast::makeConstant(Value{myList})
    );
    doAssignment(interpreter, std::move(listAssignment));

    auto discard = ast::makeDiscard(
        ast::makeVariable(Name{"myList"}),
        ast::makeConstant(Value{String{"2"}}) // not an integer!
    );

    EXPECT_THROW({
        doDiscard(interpreter, std::move(discard));
    }, std::runtime_error);
}
