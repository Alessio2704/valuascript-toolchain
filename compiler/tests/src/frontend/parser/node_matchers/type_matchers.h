#pragma once

#include "core.h"
#include "expect_functions.h"

namespace valuascript::compiler::test
{
    template <typename T = TypeVerifier>
    TypeVerifier IsType(std::string name, std::initializer_list<T>) = delete;

    template <typename... Matchers>
    struct TypeVariadicMatcher
    {
        using node_type = TypeAnnotation;
        std::string name;
        std::tuple<Matchers...> generics;

        void operator()(TypeAnnotation* t) const
        {
            std::vector<TypeVerifier> gen_vec;
            gen_vec.reserve(sizeof...(Matchers));
            std::apply([&](const auto&... m)
            {
                (gen_vec.push_back(TypeVerifier(m)), ...);
            }, generics);
            ExpectType(t, name, gen_vec);
        }
    };

    struct TypeMatcher
    {
        using node_type = TypeAnnotation;
        std::string name;
        std::vector<TypeVerifier> generics;
        void operator()(TypeAnnotation* t) const { ExpectType(t, name, generics); }
    };

    inline FluentNodeMatcher<TypeMatcher> IsType(std::string name, std::vector<TypeVerifier> generics = {})
    {
        return FluentNodeMatcher<TypeMatcher>{TypeMatcher{.name = std::move(name), .generics = std::move(generics)}};
    }

    template <typename... Matchers>
        requires (sizeof...(Matchers) > 0 && !(sizeof...(Matchers) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Matchers...>>>, std::vector<TypeVerifier>>))
    inline auto IsType(std::string name, Matchers&&... matchers)
    {
        return FluentNodeMatcher<TypeVariadicMatcher<std::decay_t<Matchers>...>>{
            TypeVariadicMatcher<std::decay_t<Matchers>...>{
                std::move(name), std::make_tuple(std::forward<Matchers>(matchers)...)
            }
        };
    }

    template <typename T = TypeVerifier>
    TypeVerifier IsTupleType(std::initializer_list<T>) = delete;

    template <typename... Matchers>
    struct TupleTypeVariadicMatcher
    {
        using node_type = TypeAnnotation;
        std::tuple<Matchers...> elements;

        void operator()(TypeAnnotation* node) const
        {
            if (auto t = ExpectNode<TupleTypeAnnotation>(node))
            {
                ASSERT_EQ(t->element_types.size(), sizeof...(Matchers)) <<
                    "TupleTypeAnnotation element count mismatch.";
                size_t idx = 0;
                std::apply([&](const auto&... m)
                {
                    ((m(t->element_types[idx++].get())), ...);
                }, elements);
            }
        }
    };

    struct TupleTypeMatcher
    {
        using node_type = TypeAnnotation;
        std::vector<TypeVerifier> elements;
        void operator()(TypeAnnotation* t) const { ExpectTupleType(t, elements); }
    };

    inline FluentNodeMatcher<TupleTypeMatcher> IsTupleType(std::vector<TypeVerifier> elements = {})
    {
        return FluentNodeMatcher<TupleTypeMatcher>{TupleTypeMatcher{std::move(elements)}};
    }

    template <typename... Matchers>
        requires (sizeof...(Matchers) > 0 && !(sizeof...(Matchers) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Matchers...>>>, std::vector<TypeVerifier>>))
    inline auto IsTupleType(Matchers&&... matchers)
    {
        return FluentNodeMatcher<TupleTypeVariadicMatcher<std::decay_t<Matchers>...>>{
            TupleTypeVariadicMatcher<std::decay_t<Matchers>...>{
                std::make_tuple(std::forward<Matchers>(matchers)...)
            }
        };
    }
}
