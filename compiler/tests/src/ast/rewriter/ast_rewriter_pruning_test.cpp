#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <utility>

#include "ast_rewriter_test_helper.h"
#include "ast/categories/ast_category_types.h"
#include "ast/utils/ast_cast.h"

namespace valuascript::compiler::test
{
    template <typename NodeT>
    inline void run_nullptr_rewrite_test()
    {
        AstRewriter rewriter;
        if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
        {
            // Inner nodes are value types and not nullable pointer inputs
        }
        else
        {
            EXPECT_EQ(rewriter.rewrite(std::unique_ptr<NodeT>(nullptr)), nullptr);
        }
    }

    template <typename CategoryT>
    inline void run_category_nullptr_rewrite_test()
    {
        AstRewriter rewriter;
        EXPECT_EQ(rewriter.rewrite(std::unique_ptr<CategoryT>(nullptr)), nullptr);
    }

    template <typename NodeT>
    struct AstRewriterNullptrTestBuilder
    {
        static AstRewriterTestDescriptor build()
        {
            return AstRewriterTestDescriptor{
                .name = std::string(get_ast_node_name<NodeT>()),
                .run_test = []()
                {
                    run_nullptr_rewrite_test<NodeT>();
                }
            };
        }
    };

    template <typename CategoryT>
    struct AstRewriterCategoryNullptrTestBuilder
    {
        static AstRewriterTestDescriptor build()
        {
            return AstRewriterTestDescriptor{
                .name = std::string(category_name_v<CategoryT>),
                .run_test = []()
                {
                    run_category_nullptr_rewrite_test<CategoryT>();
                }
            };
        }
    };

    class AstRewriterNullptrParameterizedTest : public testing::TestWithParam<AstRewriterTestDescriptor>
    {
    };

    class AstRewriterCategoryNullptrParameterizedTest : public testing::TestWithParam<AstRewriterTestDescriptor>
    {
    };

    TEST_P(AstRewriterNullptrParameterizedTest, HandlesNullptrSafely)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Nullptr Safety for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    TEST_P(AstRewriterCategoryNullptrParameterizedTest, HandlesCategoryNullptrSafely)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Category Nullptr Safety for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstRewriterNullptrParameterizedTest,
        testing::ValuesIn((AstRewriterDescriptorCollector<AllAstNodeTypes, AstRewriterNullptrTestBuilder>::collect())),
        AstRewriterTestNameGenerator{}
    );

    INSTANTIATE_TEST_SUITE_P(
        AllCategories,
        AstRewriterCategoryNullptrParameterizedTest,
        testing::ValuesIn((AstRewriterDescriptorCollector<AstCategoryTypes, AstRewriterCategoryNullptrTestBuilder>::collect())),
        AstRewriterTestNameGenerator{}
    );

    template <typename NodeT>
    inline void run_optional_pruning_test()
    {
        constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<NodeT>::members)>;

        [&]<size_t... Is>(std::index_sequence<Is...>) {
            ([&] {
                using MemberPtrT = std::tuple_element_t<Is, decltype(AstNodeSchema<NodeT>::members)>;
                using MemberRawT = std::remove_cvref_t<decltype(std::declval<NodeT>().*std::declval<MemberPtrT>())>;

                if constexpr (IsOptionalAstFieldOfAstNode<MemberRawT> || IsStdOptionalOfAstNode<MemberRawT> || IsUniquePtrOfAstNode<MemberRawT>)
                {
                    auto sample = create_sample<NodeT>(2);
                    if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
                    {
                        auto& member = sample.*std::get<Is>(AstNodeSchema<NodeT>::members);
                        member = {};
                        AstRewriter rewriter;
                        auto rewritten = rewriter.rewrite(std::move(sample));
                        auto& rewritten_member = rewritten.*std::get<Is>(AstNodeSchema<NodeT>::members);
                        if constexpr (IsUniquePtrOfAstNode<MemberRawT>)
                        {
                            EXPECT_EQ(rewritten_member, nullptr);
                        }
                        else
                        {
                            EXPECT_FALSE(rewritten_member.has_value());
                        }
                    }
                    else
                    {
                        if (sample)
                        {
                            auto& member = (*sample).*std::get<Is>(AstNodeSchema<NodeT>::members);
                            member = {};
                            AstRewriter rewriter;
                            auto rewritten = rewriter.rewrite(std::move(sample));
                            if (rewritten)
                            {
                                auto* concrete = dynamic_cast<NodeT*>(rewritten.get());
                                if (concrete)
                                {
                                    auto& rewritten_member = (*concrete).*std::get<Is>(AstNodeSchema<NodeT>::members);
                                    if constexpr (IsUniquePtrOfAstNode<MemberRawT>)
                                    {
                                        EXPECT_EQ(rewritten_member, nullptr);
                                    }
                                    else
                                    {
                                        EXPECT_FALSE(rewritten_member.has_value());
                                    }
                                }
                            }
                        }
                    }
                }
            }(), ...);
        }(std::make_index_sequence<N>{});
    }

    template <typename NodeT>
    struct AstRewriterOptionalPruningTestBuilder
    {
        static AstRewriterTestDescriptor build()
        {
            return AstRewriterTestDescriptor{
                .name = std::string(get_ast_node_name<NodeT>()),
                .run_test = []()
                {
                    run_optional_pruning_test<NodeT>();
                }
            };
        }
    };

    class AstRewriterOptionalPruningParameterizedTest : public testing::TestWithParam<AstRewriterTestDescriptor>
    {
    };

    TEST_P(AstRewriterOptionalPruningParameterizedTest, HandlesOptionalMemberPruningAcrossAllNodes)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Optional Pruning for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstRewriterOptionalPruningParameterizedTest,
        testing::ValuesIn((AstRewriterDescriptorCollector<AllAstNodeTypes, AstRewriterOptionalPruningTestBuilder>::collect())),
        AstRewriterTestNameGenerator{}
    );
}
