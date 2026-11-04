#pragma once

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <concepts>
#include <tuple>
#include <utility>

#include "token/source_span.h"
#include "ast/core/ast_core.h"
#include "ast/clone/ast_clone.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/equality/ast_disjoint.h"
#include "ast/equality/ast_equality.h"
#include "ast/factory/ast_factory.h"

namespace valuascript::compiler::test
{
    template <typename NodeT, size_t Is>
    inline void verify_member_cloned(const NodeT& original, const NodeT& cloned)
    {
        using MemberPtrT = std::tuple_element_t<Is, decltype(AstNodeSchema<NodeT>::members)>;
        using MemberRawT = std::remove_cvref_t<decltype(std::declval<NodeT>().*std::declval<MemberPtrT>())>;

        const auto& orig_member = original.*std::get<Is>(AstNodeSchema<NodeT>::members);
        const auto& clone_member_val = cloned.*std::get<Is>(AstNodeSchema<NodeT>::members);

        if constexpr (IsUniquePtrOfAstNode<MemberRawT>)
        {
            if (orig_member)
            {
                ASSERT_NE(clone_member_val, nullptr);
                EXPECT_NE(clone_member_val.get(), orig_member.get());
                EXPECT_TRUE(ast_equals(clone_member_val, orig_member));
                EXPECT_TRUE(ast_is_disjoint(orig_member, clone_member_val));
                EXPECT_TRUE(clone_member_val->is_valid());
            }
            else
            {
                EXPECT_EQ(clone_member_val, nullptr);
            }
        }
        else if constexpr (IsVectorOfUniquePtrOfAstNode<MemberRawT>)
        {
            ASSERT_EQ(clone_member_val.size(), orig_member.size());
            for (size_t i = 0; i < orig_member.size(); ++i)
            {
                ASSERT_NE(clone_member_val[i], nullptr);
                ASSERT_NE(orig_member[i], nullptr);
                EXPECT_NE(clone_member_val[i].get(), orig_member[i].get());
                EXPECT_TRUE(ast_equals(clone_member_val[i], orig_member[i]));
                EXPECT_TRUE(ast_is_disjoint(orig_member[i], clone_member_val[i]));
                EXPECT_TRUE(clone_member_val[i]->is_valid());
            }
        }
        else if constexpr (IsInnerAstNode<MemberRawT>)
        {
            EXPECT_TRUE(ast_equals(clone_member_val, orig_member));
            EXPECT_TRUE(ast_is_disjoint(orig_member, clone_member_val));
            EXPECT_TRUE(clone_member_val.is_valid());
        }
        else if constexpr (IsVectorOfInnerAstNode<MemberRawT>)
        {
            ASSERT_EQ(clone_member_val.size(), orig_member.size());
            for (size_t i = 0; i < orig_member.size(); ++i)
            {
                EXPECT_TRUE(ast_equals(clone_member_val[i], orig_member[i]));
                EXPECT_TRUE(ast_is_disjoint(orig_member[i], clone_member_val[i]));
                EXPECT_TRUE(clone_member_val[i].is_valid());
            }
        }
        else if constexpr (IsOptionalAstFieldOfAstNode<MemberRawT>)
        {
            if (orig_member.has_value())
            {
                ASSERT_TRUE(clone_member_val.has_value());
                using ValT = typename MemberRawT::value_type;
                if constexpr (is_unique_ptr_v<ValT>)
                {
                    ASSERT_NE(clone_member_val.get(), nullptr);
                    EXPECT_NE(clone_member_val.get(), orig_member.get());
                    EXPECT_TRUE(ast_equals(clone_member_val.get(), orig_member.get()));
                    EXPECT_TRUE(ast_is_disjoint(orig_member.get(), clone_member_val.get()));
                    EXPECT_TRUE(clone_member_val->is_valid());
                }
                else
                {
                    EXPECT_TRUE(ast_equals(clone_member_val.value(), orig_member.value()));
                    EXPECT_TRUE(ast_is_disjoint(orig_member.value(), clone_member_val.value()));
                    EXPECT_TRUE(clone_member_val.value().is_valid());
                }
            }
            else
            {
                EXPECT_FALSE(clone_member_val.has_value());
            }
        }
        else
        {
            EXPECT_EQ(clone_member_val, orig_member);
        }
    }

    template <typename Base, typename T>
    inline void test_polymorphic_base_clone(const T& node_instance)
    {
        if constexpr (std::derived_from<T, Base>)
        {
            auto base_clone = clone_node(static_cast<const Base*>(&node_instance));
            ASSERT_NE(base_clone, nullptr);
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
    concept HasValidCloneSampleFactory = requires
    {
        { create_sample<T>(0) };
    };

    template <typename NodeT>
    inline void run_schema_driven_clone_test()
    {
        AstFactoryConfig config;
        config.function_parameter.default_value.value = true;
        config.enum_case.value.value = true;
        config.type_annotation.generic_args.count = 2;
        config.tuple_type_annotation.generic_args.count = 2;
        config.tuple_type_annotation.element_types.count = 2;

        if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
        {
            NodeT orig = create_sample<NodeT>(2, config);
            NodeT cloned = clone_node(orig);

            EXPECT_TRUE(ast_is_clone_of(orig, cloned));

            constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<NodeT>::members)>;
            [&]<size_t... Is>(std::index_sequence<Is...>) {
                (verify_member_cloned<NodeT, Is>(orig, cloned), ...);
            }(std::make_index_sequence<N>{});

            test_polymorphic_base_clone<AstNode>(orig);
            CategoryPolymorphicTester<NodeT, AstCategoryTypes>::test(orig);

            std::vector<NodeT> vec;
            vec.push_back(create_sample<NodeT>(1, config));
            vec.push_back(create_sample<NodeT>(2, config));
            auto cloned_vec = clone_nodes(vec);
            EXPECT_TRUE(ast_is_clone_of(vec, cloned_vec));

            auto original_span = orig.span;
            cloned.span.start_offset += 1000;
            EXPECT_EQ(orig.span, original_span);
        }
        else
        {
            EXPECT_EQ(clone_node(static_cast<const NodeT*>(nullptr)), nullptr);

            auto orig = create_sample<NodeT>(2, config);
            ASSERT_NE(orig, nullptr);

            auto clone1 = clone_node(orig.get());
            auto clone2 = clone_node(orig);

            ASSERT_NE(clone1, nullptr);
            ASSERT_NE(clone2, nullptr);

            EXPECT_TRUE(ast_is_clone_of(orig, clone1));
            EXPECT_TRUE(ast_is_clone_of(orig, clone2));
            EXPECT_TRUE(ast_is_disjoint(clone1, clone2));

            constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<NodeT>::members)>;
            [&]<size_t... Is>(std::index_sequence<Is...>) {
                (verify_member_cloned<NodeT, Is>(*orig, *clone1), ...);
            }(std::make_index_sequence<N>{});

            test_polymorphic_base_clone<AstNode>(*orig);
            CategoryPolymorphicTester<NodeT, AstCategoryTypes>::test(*orig);

            std::vector<std::unique_ptr<NodeT>> vec;
            vec.push_back(create_sample<NodeT>(1, config));
            vec.push_back(create_sample<NodeT>(2, config));
            auto cloned_vec = clone_nodes(vec);
            EXPECT_TRUE(ast_is_clone_of(vec, cloned_vec));

            auto original_span = orig->span;
            clone1->span.start_offset += 1000;
            EXPECT_EQ(orig->span, original_span);
        }
    }
}
