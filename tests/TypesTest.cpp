#include <gtest/gtest.h>
#include <iostream>
#include "Types.h"

TEST(TypesTest, DiscardFromList)
{
    List<Value> list{Value{String{"a"}}, Value{String{"b"}}, Value{String{"c"}}};
    List<Value> expected{Value{String{"c"}}};

    list.discard(Integer{2});

    EXPECT_EQ(list, expected);
}

TEST(TypesTest, DiscardListSizeFromList)
{
    List<Value> list{Value{String{"a"}}, Value{String{"b"}}, Value{String{"c"}}};
    List<Value> expected{};

    list.discard(Integer{3});

    EXPECT_EQ(list, expected);
}

TEST(TypesTest, DiscardMoreThanListSizeFromList)
{
    List<Value> list{Value{String{"a"}}, Value{String{"b"}}, Value{String{"c"}}};
    List<Value> expected{};

    list.discard(Integer{5});

    EXPECT_EQ(list, expected);
}

TEST(TypesTest, DiscardNegativeAmountFromList)
{
    List<Value> list{Value{String{"a"}}};
    List<Value> expected{Value{String{"a"}}};

    list.discard(Integer{-1});

    EXPECT_EQ(list, expected);
}

TEST(TypesTest, SortListOfStrings)
{
    List<Value> list{Value{String{"b"}}, Value{String{"a"}}, Value{String{"c"}}};
    List<Value> expected{Value{String{"a"}}, Value{String{"b"}}, Value{String{"c"}}};

    List<Value> actual = sortList(list);

    EXPECT_EQ(actual, expected);
}

TEST(TypesTest, SortListOfIntegers)
{
    List<Value> list{Value{Integer{1}}, Value{Integer{-1}}, Value{Integer{0}}};
    List<Value> expected{Value{Integer{-1}}, Value{Integer{0}}, Value{Integer{1}}};

    List<Value> actual = sortList(list);

    EXPECT_EQ(actual, expected);
}

TEST(TypesTest, SortEmptyList)
{
    List<Value> list{};
    List<Value> expected{};

    List<Value> actual = sortList(list);

    EXPECT_EQ(actual, expected);
}

TEST(TypesTest, SortListOfMixedValues)
{
    List<Value> list{Value{String{"a"}}, Value{Integer{1}}};
    List<Value> expected{Value{String{"a"}}, Value{Integer{1}}};

    EXPECT_THROW({
        sortList(list);
    }, std::runtime_error);

    EXPECT_EQ(list, expected); // original list should be unchanged
}

TEST(TypesTest, SortListOfLists)
{
    List<Value> innerList1{Value{String{"a"}}};
    List<Value> innerList2{Value{String{"b"}}};
    List<Value> list{Value{innerList1}, Value{innerList2}};
    List<Value> expected{Value{innerList1}, Value{innerList2}};

    EXPECT_THROW({
        sortList(list);
    }, std::runtime_error);

    EXPECT_EQ(list, expected); // original list should be unchanged
}

TEST(TypesTest, SortListOfMapsNoKey)
{
    Map<String, Value> map1;
    Map<String, Value> map2;
    Map<String, Value> map3;
    map1.setAttribute(String{"a"}, Value{String{"200"}});
    map2.setAttribute(String{"a"}, Value{String{"100"}});
    map3.setAttribute(String{"a"}, Value{String{"300"}});

    List<Value> list{Value{map1}, Value{map2}, Value{map3}};
    List<Value> expected{Value{map1}, Value{map2}, Value{map3}};

    EXPECT_THROW({
        sortList(list);
    }, std::runtime_error);

    EXPECT_EQ(list, expected); // original list should be unchanged
}

TEST(TypesTest, SortListOfMapsOfStringValuesWithKey)
{
    Map<String, Value> map1;
    Map<String, Value> map2;
    Map<String, Value> map3;
    map1.setAttribute(String{"a"}, Value{String{"200"}});
    map2.setAttribute(String{"a"}, Value{String{"100"}});
    map3.setAttribute(String{"a"}, Value{String{"300"}});

    List<Value> list{Value{map1}, Value{map2}, Value{map3}};
    List<Value> expected{Value{map2}, Value{map1}, Value{map3}};

    List<Value> actual = sortList(list, String{"a"});

    EXPECT_EQ(actual, expected);
}

TEST(TypesTest, SortListOfMapsOfIntegerValuesWithKey)
{
    Map<String, Value> map1;
    Map<String, Value> map2;
    Map<String, Value> map3;
    map1.setAttribute(String{"damage"}, Value{Integer{90}});
    map2.setAttribute(String{"damage"}, Value{Integer{10}});
    map3.setAttribute(String{"damage"}, Value{Integer{30}});

    List<Value> list{Value{map1}, Value{map2}, Value{map3}};
    List<Value> expected{Value{map2}, Value{map3}, Value{map1}};

    List<Value> actual = sortList(list, String{"damage"});

    EXPECT_EQ(actual, expected);
}

TEST(TypesTest, SortListOfMapsOfMixedValuesWithKey)
{
    Map<String, Value> map1;
    Map<String, Value> map2;
    Map<String, Value> map3;
    map1.setAttribute(String{"a"}, Value{String{"200"}});
    map2.setAttribute(String{"a"}, Value{Integer{100}});
    map3.setAttribute(String{"a"}, Value{String{"300"}});

    List<Value> list{Value{map1}, Value{map2}, Value{map3}};
    List<Value> expected{Value{map1}, Value{map2}, Value{map3}};

    EXPECT_THROW({
        sortList(list, String{"a"});
    }, std::runtime_error);

    EXPECT_EQ(list, expected); // original list should be unchanged
}

TEST(TypesTest, SortListOfMapsWithKeyMissing)
{
    Map<String, Value> map1;
    Map<String, Value> map2;
    Map<String, Value> map3;
    map1.setAttribute(String{"a"}, Value{String{"200"}});
    map2.setAttribute(String{"a"}, Value{String{"100"}});
    map3.setAttribute(String{"z"}, Value{String{"300"}}); // doesn't have "a" key

    List<Value> list{Value{map1}, Value{map2}, Value{map3}};
    List<Value> expected{Value{map1}, Value{map2}, Value{map3}};

    EXPECT_THROW({
        sortList(list, String{"a"});
    }, std::runtime_error);

    EXPECT_EQ(list, expected); // original list should be unchanged
}

using CompareTestParams = std::tuple<Value, Value, std::optional<bool>>;

class CompareValuesTest : public ::testing::TestWithParam<CompareTestParams> {};

TEST_P(CompareValuesTest, ComparesValues)
{
    Value left = std::get<0>(GetParam());
    Value right = std::get<1>(GetParam());
    auto expected = std::get<2>(GetParam());

    auto actual = maybeCompareValues(left, right);
    EXPECT_EQ(actual, expected);
}

INSTANTIATE_TEST_SUITE_P(
    TypesTest,
    CompareValuesTest,
    ::testing::Values(
        std::make_tuple(Value{String{"a"}}, Value{String{"b"}}, true),
        std::make_tuple(Value{String{"a"}}, Value{String{"a"}}, false),
        std::make_tuple(Value{String{"b"}}, Value{String{"a"}}, false),
        std::make_tuple(Value{Integer{99}}, Value{Integer{100}}, true),
        std::make_tuple(Value{Integer{100}}, Value{Integer{100}}, false),
        std::make_tuple(Value{Integer{101}}, Value{Integer{100}}, false),
        std::make_tuple(Value{Boolean{false}}, Value{Boolean{true}}, true),
        std::make_tuple(Value{Boolean{true}}, Value{Boolean{false}}, false),
        // Comparing mixed types isn't supported
        std::make_tuple(Value{String{"1"}}, Value{Integer{1}}, std::nullopt),
        std::make_tuple(Value{String{"1"}}, Value{Boolean{true}}, std::nullopt),
        std::make_tuple(Value{Integer{1}}, Value{Boolean{true}}, std::nullopt),
        // Comparing Lists isn't supported
        std::make_tuple(Value{List<Value>{}}, Value{List<Value>{}}, std::nullopt),
        // Comparing Maps isn't supported
        std::make_tuple(Value{Map<String, Value>{}}, Value{Map<String, Value>{}}, std::nullopt)
    )
);
