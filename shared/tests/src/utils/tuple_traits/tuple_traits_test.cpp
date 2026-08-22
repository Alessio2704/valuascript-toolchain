#include <gtest/gtest.h>
#include <tuple>
#include <string>
#include "utils/tuple_traits.h"

namespace valuascript::shared::test
{
    TEST(TupleTraitsTest, TupleContainsTypeBasic)
    {
        using TestTuple = std::tuple<int, double, std::string>;
        static_assert(tuple_contains_type_v<int, TestTuple>, "TestTuple must contain int");
        static_assert(tuple_contains_type_v<double, TestTuple>, "TestTuple must contain double");
        static_assert(tuple_contains_type_v<std::string, TestTuple>, "TestTuple must contain std::string");
        static_assert(!tuple_contains_type_v<char, TestTuple>, "TestTuple must not contain char");
        static_assert(!tuple_contains_type_v<void, TestTuple>, "TestTuple must not contain void");
        static_assert(!tuple_contains_type_v<int, std::tuple<>>, "Empty tuple must not contain int");

        EXPECT_TRUE((tuple_contains_type_v<int, TestTuple>));
        EXPECT_FALSE((tuple_contains_type_v<char, TestTuple>));
    }

    TEST(TupleTraitsTest, TupleCountTypeBasic)
    {
        using UniqueTuple = std::tuple<int, double, std::string>;
        using DuplicateTuple = std::tuple<int, double, int, std::string, int>;

        static_assert(tuple_count_type_v<int, UniqueTuple> == 1, "UniqueTuple must contain exactly 1 int");
        static_assert(tuple_count_type_v<char, UniqueTuple> == 0, "UniqueTuple must contain 0 char");
        static_assert(tuple_count_type_v<int, DuplicateTuple> == 3, "DuplicateTuple must contain 3 int instances");
        static_assert(tuple_count_type_v<double, DuplicateTuple> == 1, "DuplicateTuple must contain 1 double instance");
        static_assert(tuple_count_type_v<void, std::tuple<>> == 0, "Empty tuple must contain 0 void");

        EXPECT_EQ((tuple_count_type_v<int, DuplicateTuple>), 3u);
    }

    TEST(TupleTraitsTest, IsTupleUniqueBasic)
    {
        using EmptyTuple = std::tuple<>;
        using SingleTuple = std::tuple<int>;
        using UniqueTuple = std::tuple<int, double, std::string, char>;
        using DuplicateTuple = std::tuple<int, double, int, std::string>;

        static_assert(is_tuple_unique_v<EmptyTuple>, "EmptyTuple must be unique");
        static_assert(is_tuple_unique_v<SingleTuple>, "SingleTuple must be unique");
        static_assert(is_tuple_unique_v<UniqueTuple>, "UniqueTuple must be unique");
        static_assert(!is_tuple_unique_v<DuplicateTuple>, "DuplicateTuple must not be unique");

        EXPECT_TRUE(is_tuple_unique_v<UniqueTuple>);
        EXPECT_FALSE(is_tuple_unique_v<DuplicateTuple>);
    }

    TEST(TupleTraitsTest, TupleIndexOfBasic)
    {
        using TestTuple = std::tuple<int, double, std::string, char>;

        static_assert(tuple_index_of_v<int, TestTuple> == 0, "int index in TestTuple must be 0");
        static_assert(tuple_index_of_v<double, TestTuple> == 1, "double index in TestTuple must be 1");
        static_assert(tuple_index_of_v<std::string, TestTuple> == 2, "std::string index in TestTuple must be 2");
        static_assert(tuple_index_of_v<char, TestTuple> == 3, "char index in TestTuple must be 3");

        EXPECT_EQ((tuple_index_of_v<int, TestTuple>), 0u);
        EXPECT_EQ((tuple_index_of_v<std::string, TestTuple>), 2u);
    }
}
