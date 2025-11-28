#include <gtest/gtest.h>
#include <optional>
#include <iostream>

#include "Helpers.h"
#include "GameInterpreter.h"

TEST(AssignmentTest, AssignStringToVariable)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    auto assignment = ast::makeAssignment(
        ast::makeVariable(Name{"score"}),
        ast::makeConstant(Value{String{"100"}})
    );
    doAssignment(interpreter, std::move(assignment));

    Value storedVar = loadVariable(interpreter, Name{"score"});
    EXPECT_EQ(storedVar, Value{String{"100"}});
}

TEST(AssignmentTest, AssignMapToVariable)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    Map<String, Value> map{};
    map.setAttribute(String{"a"}, Value{String{"1"}});

    auto varAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"myMap"}),
        ast::makeConstant(Value{map})
    );
    doAssignment(interpreter, std::move(varAssignment));

    Value storedVar = loadVariable(interpreter, Name{"myMap"});
    EXPECT_EQ(storedVar.getAttribute(String{"a"}), Value{String{"1"}});
}

TEST(AssignmentTest, AssignVariableMapAttribute)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    Map<String, Value> map{};

    auto varAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"myMap"}),
        ast::makeConstant(Value{map})
    );
    doAssignment(interpreter, std::move(varAssignment));

    Value storedVar = loadVariable(interpreter, Name{"myMap"});
    EXPECT_EQ(storedVar.asMap().value.size(), 0);

    auto attrAssignment  = ast::makeAssignment(
        ast::makeAttribute(
            ast::makeVariable(
                Name{"myMap"}
            ),
            String{"a"}
        ),
        ast::makeConstant(Value{String{"1"}})
    );
    doAssignment(interpreter, std::move(attrAssignment));

    storedVar = loadVariable(interpreter, Name{"myMap"});
    EXPECT_EQ(storedVar.asMap().value.size(), 1);
    EXPECT_EQ(storedVar.getAttribute(String{"a"}), Value{String{"1"}});
}

TEST(AssignmentTest, AssignVariableNestedMapAttribute)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    Map<String, Value> map{};
    Map<String, Value> nestedMap{};
    nestedMap.setAttribute(String{"a"}, Value{String{"1"}});
    map.setAttribute(String{"myNestedMap"}, Value{nestedMap});

    auto varAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"myMap"}),
        ast::makeConstant(Value{map})
    );
    doAssignment(interpreter, std::move(varAssignment));

    auto loadedMap = loadVariable(interpreter, Name{"myMap"}).asMap();
    auto loadedNestedMap = loadedMap.getAttribute(String{"myNestedMap"}).asMap();
    EXPECT_EQ(loadedNestedMap.getAttribute(String{"a"}), Value{String{"1"}});

    auto nestedAttrAssignment = ast::makeAssignment(
        ast::makeAttribute(
            ast::makeAttribute(
                ast::makeVariable(Name{"myMap"}),
                String{"myNestedMap"}
            ),
            String{"a"}
        ),
        ast::makeConstant(Value{String{"2"}})
    );
    doAssignment(interpreter, std::move(nestedAttrAssignment));

    loadedMap = loadVariable(interpreter, Name{"myMap"}).asMap();
    loadedNestedMap = loadedMap.getAttribute(String{"myNestedMap"}).asMap();
    EXPECT_EQ(loadedNestedMap.getAttribute(String{"a"}), Value{String{"2"}});
}

TEST(AssignmentTest, AssignVariableToVariableString)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    auto var1Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var1"}),
        ast::makeConstant(Value{String{"a"}})
    );
    auto var2Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var2"}),
        ast::makeConstant(Value{String{"b"}})
    );
    doAssignment(interpreter, std::move(var1Assignment));
    doAssignment(interpreter, std::move(var2Assignment));

    Value storedVar1 = loadVariable(interpreter, Name{"var1"});
    Value storedVar2 = loadVariable(interpreter, Name{"var2"});
    EXPECT_EQ(storedVar1, Value{String{"a"}});
    EXPECT_EQ(storedVar2, Value{String{"b"}});

    auto var1ToVar2Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var1"}),
        ast::makeVariable(Name{"var2"})
    );
    doAssignment(interpreter, std::move(var1ToVar2Assignment));

    storedVar1 = loadVariable(interpreter, Name{"var1"});
    storedVar2 = loadVariable(interpreter, Name{"var2"});
    EXPECT_EQ(storedVar1, Value{String{"b"}});
    EXPECT_EQ(storedVar2, Value{String{"b"}});

    // Check - does var1 hold a copy or reference of var2's value?
    auto var2Modify = ast::makeAssignment(
        ast::makeVariable(Name{"var2"}),
        ast::makeConstant(Value{String{"c"}})
    );
    doAssignment(interpreter, std::move(var2Modify));

    // var1 should not reflect the changes made to var2 if it was a copy
    storedVar1 = loadVariable(interpreter, Name{"var1"});
    storedVar2 = loadVariable(interpreter, Name{"var2"});
    EXPECT_EQ(storedVar1, Value{String{"b"}});
    EXPECT_EQ(storedVar2, Value{String{"c"}});
}

TEST(AssignmentTest, AssignVariableToVariableMap)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});

    Map<String, Value> map;
    map.setAttribute(String{"a"}, Value{String{"1"}});

    auto var2Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var2"}),
        ast::makeConstant(Value{map})
    );
    doAssignment(interpreter, std::move(var2Assignment));

    Value storedVar1;
    Value storedVar2 = loadVariable(interpreter, Name{"var2"});
    EXPECT_EQ(storedVar2.getAttribute(String{"a"}), Value{String{"1"}});

    auto var1ToVar2Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var1"}),
        ast::makeVariable(Name{"var2"})
    );
    doAssignment(interpreter, std::move(var1ToVar2Assignment));

    storedVar1 = loadVariable(interpreter, Name{"var1"});
    storedVar2 = loadVariable(interpreter, Name{"var2"});
    EXPECT_EQ(storedVar1.getAttribute(String{"a"}), Value{String{"1"}});
    EXPECT_EQ(storedVar2.getAttribute(String{"a"}), Value{String{"1"}});

    // Check - does var1 hold a copy or reference of var2's value?
    auto var2Modify = ast::makeAssignment(
        ast::makeAttribute(
            ast::makeVariable(Name{"var2"}),
            String{"a"}
        ),
        ast::makeConstant(Value{String{"2"}})
    );
    doAssignment(interpreter, std::move(var2Modify));

    // var1 should not reflect the changes made to var2 if it was a copy
    storedVar1 = loadVariable(interpreter, Name{"var1"});
    storedVar2 = loadVariable(interpreter, Name{"var2"});
    EXPECT_EQ(storedVar1.getAttribute(String{"a"}), Value{String{"1"}});
    EXPECT_EQ(storedVar2.getAttribute(String{"a"}), Value{String{"2"}});
}

TEST(AssignmentTest, AssignConstant)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});
    auto invalidAssignment = ast::makeAssignment(
        ast::makeConstant(Value{String{"a"}}), // Invalid syntax, constants are not assignable
        ast::makeConstant(Value{String{"b"}})
    );

    EXPECT_THROW({
        doAssignment(interpreter, std::move(invalidAssignment));
    }, std::runtime_error);
}

TEST(AssignmentTest, AssignUndefinedVariable)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});
    auto invalidAssignment = ast::makeAssignment(
        ast::makeVariable(Name{"var1"}),
        ast::makeVariable(Name{"var2"}) // var2 is undefined
    );

    EXPECT_THROW({
        doAssignment(interpreter, std::move(invalidAssignment));
    }, std::runtime_error);
}

TEST(AssignmentTest, AssignUndefinedAttribute)
{
    InputManager inputManager;
    GameInterpreter interpreter(inputManager, {});
    Map<String, Value> map;
    map.setAttribute(String{"a"}, Value{String{"1"}});

    auto var1Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var1"}),
        ast::makeConstant(Value{map})
    );
    doAssignment(interpreter, std::move(var1Assignment));

    auto invalidVar2Assignment = ast::makeAssignment(
        ast::makeVariable(Name{"var2"}),
        ast::makeAttribute(
            ast::makeVariable(Name{"var1"}),
            String{"b"} // var1 doesn't have a "b" attribute
        )
    );

    EXPECT_THROW({
        doAssignment(interpreter, std::move(invalidVar2Assignment));
    }, std::runtime_error);
}
