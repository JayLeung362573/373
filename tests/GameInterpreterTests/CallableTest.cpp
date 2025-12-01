#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"


TEST(CallableTest, Size)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    List<Value> list{Value{String{"a"}}, Value{String{"b"}}};

    auto listSizeAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"listSize"}),
        ast::makeCallable(
            ast::makeConstant(Value{list}),
            {},
            ast::Callable::Kind::SIZE
        )
    );
    doAssignment(interpreter, std::move(listSizeAssignment));

    EXPECT_EQ(
        loadVariable(interpreter, Name{"listSize"}).asInteger(),
        Integer{2}
    );
}

TEST(CallableTest, SizeNotList)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    ast::ExpressionsBuilder expressionsBuilder;

    auto listSizeAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"listSize"}),
        ast::makeCallable(
            ast::makeConstant(Value{String{"not-a-list"}}),
            {},
            ast::Callable::Kind::SIZE
        )
    );

    EXPECT_THROW({
        doAssignment(interpreter, std::move(listSizeAssignment));
    }, std::runtime_error);
}

TEST(CallableTest, SizeArgCountMismatch)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    ast::ExpressionsBuilder expressionsBuilder;

    List<Value> list{Value{String{"a"}}, Value{String{"b"}}};

    auto listSizeAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"listSize"}),
        ast::makeCallable(
            ast::makeConstant(Value{list}),
            expressionsBuilder.addExpression(
                ast::makeConstant(Value{String{"extra"}})
            ).build(),
            ast::Callable::Kind::SIZE
        )
    );

    EXPECT_THROW({
        doAssignment(interpreter, std::move(listSizeAssignment));
    }, std::runtime_error);
}

TEST(CallableTest, UpFrom)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    ast::ExpressionsBuilder expressionsBuilder;

    List<Value> expectedList{
        Value{Integer{1}},
        Value{Integer{2}},
        Value{Integer{3}},
    };

    auto listSizeAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"list"}),
        ast::makeCallable(
            ast::makeConstant(Value{Integer{3}}), // to
            expressionsBuilder.addExpression(
                ast::makeConstant(Value{Integer{1}}) // from
            ).build(),
            ast::Callable::Kind::UP_FROM
        )
    );
    doAssignment(interpreter, std::move(listSizeAssignment));

    EXPECT_EQ(
        loadVariable(interpreter, Name{"list"}).asList(),
        expectedList
    );
}
