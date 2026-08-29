#pragma once

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <concepts>
#include <tuple>

#include "token/source_span.h"
#include "ast/core/ast_core.h"
#include "ast/clone/ast_clone.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/equality/ast_disjoint.h"
#include "ast/factory/ast_factory.h"

namespace valuascript::compiler::test
{
    template <typename Base, typename T>
    inline void test_polymorphic_base_clone(const T& node_instance)
    {
        if constexpr (std::derived_from<T, Base>)
        {
            auto base_clone = clone_node(static_cast<const Base*>(&node_instance));
            EXPECT_TRUE(is_a<T>(base_clone.get()));
            EXPECT_TRUE(ast_is_clone_of(&node_instance, base_clone.get()));
        }
    }

    template <typename T, typename CategoryTuple>
    struct CategoryPolymorphicTester;

    template <typename T, typename... Categories>
    struct CategoryPolymorphicTester<T, std::tuple<Categories...>>
    {
        static void test(const T& target_node)
        {
            (test_polymorphic_base_clone<Categories>(target_node), ...);
        }
    };

    template <typename T>
    struct AstCloneSampleFactory;

    template <typename T>
    concept HasValidCloneSampleFactory = requires
    {
        { AstCloneSampleFactory<T>::create_sample() };
        { AstCloneSampleFactory<T>::run_full_clone_test() } -> std::same_as<void>;
    };

    template <typename T>
    inline void run_pointer_node_test()
    {
        EXPECT_EQ(clone_node(static_cast<const T*>(nullptr)), nullptr);

        auto orig = AstCloneSampleFactory<T>::create_sample();
        auto clone1 = clone_node(orig.get());
        auto clone2 = clone_node(orig);

        EXPECT_TRUE(ast_is_clone_of(orig, clone1));
        EXPECT_TRUE(ast_is_clone_of(orig, clone2));
        EXPECT_TRUE(ast_is_disjoint(clone1, clone2));

        test_polymorphic_base_clone<AstNode>(*orig);
        CategoryPolymorphicTester<T, AstCategoryTypes>::test(*orig);

        std::vector<std::unique_ptr<T>> vec;
        vec.push_back(AstCloneSampleFactory<T>::create_sample());
        vec.push_back(AstCloneSampleFactory<T>::create_sample());
        EXPECT_TRUE(ast_is_clone_of(vec, clone_nodes(vec)));

        auto original_span = orig->span;
        clone1->span.start_offset += 1000;
        EXPECT_EQ(orig->span, original_span);
    }

    template <typename T>
    inline void run_value_node_test()
    {
        T orig = AstCloneSampleFactory<T>::create_sample();
        T cloned = clone_node(orig);
        EXPECT_TRUE(ast_is_clone_of(orig, cloned));

        test_polymorphic_base_clone<AstNode>(orig);
        CategoryPolymorphicTester<T, AstCategoryTypes>::test(orig);

        std::vector<T> vec;
        vec.push_back(AstCloneSampleFactory<T>::create_sample());
        vec.push_back(AstCloneSampleFactory<T>::create_sample());
        EXPECT_TRUE(ast_is_clone_of(vec, clone_nodes(vec)));

        auto original_span = orig.span;
        cloned.span.start_offset += 1000;
        EXPECT_EQ(orig.span, original_span);
    }

    template <typename T>
    struct AstCloneSampleFactory
    {
        static auto create_sample()
        {
            return AstFactory<T>::create(0);
        }

        static void run_full_clone_test()
        {
            if constexpr (std::same_as<decltype(AstFactory<T>::create(0)), std::unique_ptr<T>>)
            {
                run_pointer_node_test<T>();
            }
            else
            {
                run_value_node_test<T>();
            }
        }
    };
}
