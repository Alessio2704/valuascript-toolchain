#pragma once

#include <tuple>
#include <type_traits>
#include <concepts>
#include <cstddef>

namespace valuascript::shared
{
    template <typename T, typename Tuple>
    struct TupleContainsType;

    template <typename T>
    struct TupleContainsType<T, std::tuple<>> : std::false_type {};

    template <typename T, typename Head, typename... Tail>
    struct TupleContainsType<T, std::tuple<Head, Tail...>>
        : std::conditional_t<std::is_same_v<T, Head>, std::true_type, TupleContainsType<T, std::tuple<Tail...>>> {};

    template <typename T, typename Tuple>
    inline constexpr bool tuple_contains_type_v = TupleContainsType<T, Tuple>::value;

    template <typename T, typename Tuple>
    concept TupleContains = tuple_contains_type_v<T, Tuple>;

    template <typename T, typename Tuple>
    struct TupleCountType;

    template <typename T>
    struct TupleCountType<T, std::tuple<>>
    {
        static constexpr size_t value = 0;
    };

    template <typename T, typename Head, typename... Tail>
    struct TupleCountType<T, std::tuple<Head, Tail...>>
    {
        static constexpr size_t value = (std::is_same_v<T, Head> ? 1 : 0) + TupleCountType<T, std::tuple<Tail...>>::value;
    };

    template <typename T, typename Tuple>
    inline constexpr size_t tuple_count_type_v = TupleCountType<T, Tuple>::value;

    template <typename Tuple>
    struct IsTupleUnique;

    template <>
    struct IsTupleUnique<std::tuple<>> : std::true_type {};

    template <typename Head, typename... Tail>
    struct IsTupleUnique<std::tuple<Head, Tail...>>
        : std::conditional_t<
            tuple_contains_type_v<Head, std::tuple<Tail...>>,
            std::false_type,
            IsTupleUnique<std::tuple<Tail...>>
        > {};

    template <typename Tuple>
    inline constexpr bool is_tuple_unique_v = IsTupleUnique<Tuple>::value;

    template <typename Tuple>
    concept UniqueTuple = is_tuple_unique_v<Tuple>;

    template <typename Tuple, size_t ExpectedSize>
    concept TupleHasSize = (std::tuple_size_v<Tuple> == ExpectedSize);

    namespace detail
    {
        template <size_t Index, typename T, typename... Types>
        struct TupleIndexOfHelper;

        template <size_t Index, typename T, typename Head, typename... Tail>
        struct TupleIndexOfHelper<Index, T, Head, Tail...>
        {
            static constexpr size_t value = std::is_same_v<T, Head>
                ? Index
                : TupleIndexOfHelper<Index + 1, T, Tail...>::value;
        };

        template <size_t Index, typename T>
        struct TupleIndexOfHelper<Index, T>
        {
            static constexpr size_t value = static_cast<size_t>(-1);
        };
    }

    template <typename T, typename Tuple>
    struct TupleIndexOf;

    template <typename T, typename... Types>
    struct TupleIndexOf<T, std::tuple<Types...>>
    {
        static_assert(tuple_contains_type_v<T, std::tuple<Types...>>, "Type not found in tuple!");
        static constexpr size_t value = detail::TupleIndexOfHelper<0, T, Types...>::value;
    };

    template <typename T, typename Tuple>
    inline constexpr size_t tuple_index_of_v = TupleIndexOf<T, Tuple>::value;
}
