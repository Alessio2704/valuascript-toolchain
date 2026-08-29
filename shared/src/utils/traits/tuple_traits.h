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

    template <typename... Tuples>
    struct TupleConcat;

    template <>
    struct TupleConcat<>
    {
        using type = std::tuple<>;
    };

    template <typename Tuple>
    struct TupleConcat<Tuple>
    {
        using type = Tuple;
    };

    template <typename... T1, typename... T2, typename... Rest>
    struct TupleConcat<std::tuple<T1...>, std::tuple<T2...>, Rest...>
    {
        using type = typename TupleConcat<std::tuple<T1..., T2...>, Rest...>::type;
    };

    template <typename... Tuples>
    using tuple_concat_t = typename TupleConcat<Tuples...>::type;

    template <typename Tuple, typename Fn>
    constexpr void tuple_for_each_type(Fn&& fn)
    {
        []<typename... Types>(std::tuple<Types...>*, auto&& f) {
            (f.template operator()<Types>(), ...);
        }(static_cast<Tuple*>(nullptr), std::forward<Fn>(fn));
    }

    template <typename Base, typename Tuple>
    struct TupleAllDeriveFrom;

    template <typename Base, typename... Types>
    struct TupleAllDeriveFrom<Base, std::tuple<Types...>>
    {
        static constexpr bool value = (std::is_base_of_v<Base, Types> && ...);
    };

    template <typename Base, typename Tuple>
    inline constexpr bool tuple_all_derive_from_v = TupleAllDeriveFrom<Base, Tuple>::value;

    template <typename TupleA, typename TupleB>
    struct AreTuplesEquivalent;

    template <typename... TypesA, typename TupleB>
    struct AreTuplesEquivalent<std::tuple<TypesA...>, TupleB>
    {
        static constexpr bool value = (tuple_contains_type_v<TypesA, TupleB> && ...) &&
                                      (std::tuple_size_v<std::tuple<TypesA...>> == std::tuple_size_v<TupleB>);
    };

    template <typename TupleA, typename TupleB>
    inline constexpr bool are_tuples_equivalent_v = AreTuplesEquivalent<TupleA, TupleB>::value;

    template <auto Tag, typename Tuple>
    struct TupleContainsTag;

    template <auto Tag>
    struct TupleContainsTag<Tag, std::tuple<>> : std::false_type {};

    template <auto Tag, typename Head, typename... Tail>
    struct TupleContainsTag<Tag, std::tuple<Head, Tail...>>
        : std::conditional_t<Head::KIND == Tag, std::true_type, TupleContainsTag<Tag, std::tuple<Tail...>>> {};

    template <auto Tag, typename Tuple>
    inline constexpr bool tuple_contains_tag_v = TupleContainsTag<Tag, Tuple>::value;

    template <typename Tuple>
    struct AreTupleTagsUnique;

    template <>
    struct AreTupleTagsUnique<std::tuple<>> : std::true_type {};

    template <typename Head, typename... Tail>
    struct AreTupleTagsUnique<std::tuple<Head, Tail...>>
        : std::conditional_t<
            TupleContainsTag<Head::KIND, std::tuple<Tail...>>::value,
            std::false_type,
            AreTupleTagsUnique<std::tuple<Tail...>>
        > {};

    template <typename Tuple>
    inline constexpr bool are_tuple_tags_unique_v = AreTupleTagsUnique<Tuple>::value;

    template <typename Tuple>
    concept UniqueTupleTags = are_tuple_tags_unique_v<Tuple>;
}
