#include <gtest/gtest.h>
#include <tuple>
#include <string>
#include "utils/tuple_traits.h"

namespace valuascript::shared::test
{
    TEST(TupleTraitsTest, TupleContainsTypeBasic)
    {
        using TestTuple = std::tuple<int, double, std::string>;
        static_assert(tuple_contains_type_v<int, TestTuple>);
        static_assert(tuple_contains_type_v<double, TestTuple>);
        static_assert(tuple_contains_type_v<std::string, TestTuple>);
        static_assert(!tuple_contains_type_v<char, TestTuple>);
        static_assert(!tuple_contains_type_v<void, TestTuple>);
        static_assert(!tuple_contains_type_v<int, std::tuple<>>);

        EXPECT_TRUE((tuple_contains_type_v<int, TestTuple>));
        EXPECT_FALSE((tuple_contains_type_v<char, TestTuple>));
    }

    TEST(TupleTraitsTest, TupleCountTypeBasic)
    {
        using UniqueTuple = std::tuple<int, double, std::string>;
        using DuplicateTuple = std::tuple<int, double, int, std::string, int>;

        static_assert(tuple_count_type_v<int, UniqueTuple> == 1);
        static_assert(tuple_count_type_v<char, UniqueTuple> == 0);
        static_assert(tuple_count_type_v<int, DuplicateTuple> == 3);
        static_assert(tuple_count_type_v<double, DuplicateTuple> == 1);
        static_assert(tuple_count_type_v<void, std::tuple<>> == 0);

        EXPECT_EQ((tuple_count_type_v<int, DuplicateTuple>), 3u);
    }

    TEST(TupleTraitsTest, IsTupleUniqueBasic)
    {
        using EmptyTuple = std::tuple<>;
        using SingleTuple = std::tuple<int>;
        using UniqueTuple = std::tuple<int, double, std::string, char>;
        using DuplicateTuple = std::tuple<int, double, int, std::string>;

        static_assert(is_tuple_unique_v<EmptyTuple>);
        static_assert(is_tuple_unique_v<SingleTuple>);
        static_assert(is_tuple_unique_v<UniqueTuple>);
        static_assert(!is_tuple_unique_v<DuplicateTuple>);

        EXPECT_TRUE(is_tuple_unique_v<UniqueTuple>);
        EXPECT_FALSE(is_tuple_unique_v<DuplicateTuple>);
    }

    TEST(TupleTraitsTest, TupleIndexOfBasic)
    {
        using TestTuple = std::tuple<int, double, std::string, char>;

        static_assert(tuple_index_of_v<int, TestTuple> == 0);
        static_assert(tuple_index_of_v<double, TestTuple> == 1);
        static_assert(tuple_index_of_v<std::string, TestTuple> == 2);
        static_assert(tuple_index_of_v<char, TestTuple> == 3);

        EXPECT_EQ((tuple_index_of_v<int, TestTuple>), 0u);
        EXPECT_EQ((tuple_index_of_v<std::string, TestTuple>), 2u);
    }
}
