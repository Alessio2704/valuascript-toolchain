#include <gtest/gtest.h>
#include <functional>
#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <array>
#include <type_traits>
#include <utility>

#include "ast/factory/ast_factory.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/categories/ast_category_types.h"

namespace valuascript::compiler::test
{
    struct AstFactoryTestDescriptor
    {
        std::string node_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstFactoryTestDescriptor& desc)
        {
            return os << desc.node_name;
        }
    };

    template <typename T>
    inline void test_single_factory_node()
    {
        constexpr std::array<int, 5> test_depths{0, 1, 2, 5, 10};
        for (int depth : test_depths)
        {
            auto sample = create_sample<T>(depth);
            if constexpr (std::same_as<decltype(sample), std::unique_ptr<T>>)
            {
                ASSERT_NE(sample, nullptr);
                EXPECT_EQ(sample->kind, T::KIND);
                EXPECT_TRUE(sample->is_valid());
            }
            else
            {
                EXPECT_EQ(sample.kind, T::KIND);
                EXPECT_TRUE(sample.is_valid());
            }

            const auto& node_ref = [&]() -> const T&
            {
                if constexpr (std::same_as<decltype(sample), std::unique_ptr<T>>)
                {
                    return *sample;
                }
                else
                {
                    return sample;
                }
            }();

            const size_t expected_col_start = (static_cast<size_t>(depth) * 4) + 1;
            const size_t expected_col_end = (static_cast<size_t>(depth) * 4) + 13;
            EXPECT_EQ(node_ref.span.column_start, expected_col_start);
            EXPECT_EQ(node_ref.span.column_end, expected_col_end);
            EXPECT_TRUE(node_ref.span.is_valid());

            const size_t min_child_col_start = ((static_cast<size_t>(depth) + 1) * 4) + 1;
            for_each_ast_member(node_ref, [&](const auto& member)
            {
                using MemberType = std::remove_cvref_t<decltype(member)>;
                if constexpr (std::same_as<MemberType, SourceSpan> || std::same_as<MemberType, NodeName>)
                {
                    return;
                }
                else if constexpr (requires { member.empty(); member.size(); })
                {
                    for (const auto& item : member)
                    {
                        if constexpr (requires { item->span; })
                        {
                            if (item != nullptr)
                            {
                                EXPECT_GE(item->span.column_start, min_child_col_start);
                            }
                        }
                        else if constexpr (requires { item.span; })
                        {
                            if constexpr (!std::same_as<std::remove_cvref_t<decltype(item)>, NodeName>)
                            {
                                EXPECT_GE(item.span.column_start, min_child_col_start);
                            }
                        }
                    }
                }
                else if constexpr (requires { member->span; })
                {
                    if (member != nullptr)
                    {
                        EXPECT_GE(member->span.column_start, min_child_col_start);
                    }
                }
            });

            auto single = sample_node<T>(depth);
            if constexpr (std::same_as<decltype(single), std::unique_ptr<T>>)
            {
                ASSERT_NE(single, nullptr);
                EXPECT_EQ(single->kind, T::KIND);
                EXPECT_TRUE(single->is_valid());
            }
            else
            {
                EXPECT_EQ(single.kind, T::KIND);
                EXPECT_TRUE(single.is_valid());
            }

            auto typed = sample_node_as<T>(depth);
            if constexpr (std::same_as<decltype(typed), std::unique_ptr<T>>)
            {
                ASSERT_NE(typed, nullptr);
                EXPECT_EQ(typed->kind, T::KIND);
                EXPECT_TRUE(typed->is_valid());
            }
            else
            {
                EXPECT_EQ(typed.kind, T::KIND);
                EXPECT_TRUE(typed.is_valid());
            }
        }

        constexpr std::array<size_t, 3> counts{0, 1, 3};
        for (size_t count : counts)
        {
            auto vec = sample_nodes<T>(count, 0);
            EXPECT_EQ(vec.size(), count);
            for (const auto& item : vec)
            {
                if constexpr (std::same_as<std::remove_cvref_t<decltype(item)>, std::unique_ptr<T>>)
                {
                    ASSERT_NE(item, nullptr);
                    EXPECT_EQ(item->kind, T::KIND);
                    EXPECT_TRUE(item->is_valid());
                }
                else
                {
                    EXPECT_EQ(item.kind, T::KIND);
                    EXPECT_TRUE(item.is_valid());
                }
            }
        }

        if constexpr (valuascript::shared::tuple_contains_type_v<T, AllExpressionNodeTypes>)
        {
            auto expr_disp = sample_expr_by_kind(ExpressionKind::from<T>(), 0);
            ASSERT_NE(expr_disp, nullptr);
            EXPECT_EQ(expr_disp->kind, T::KIND);
            EXPECT_TRUE(expr_disp->is_valid());

            auto expr_as = sample_expr_as<T>(0);
            ASSERT_NE(expr_as, nullptr);
            EXPECT_EQ(expr_as->kind, T::KIND);
            EXPECT_TRUE(expr_as->is_valid());

            auto cat_disp = sample_node_by_kind<Expression>(T::KIND, 0);
            ASSERT_NE(cat_disp, nullptr);
            EXPECT_EQ(cat_disp->kind, T::KIND);
            EXPECT_TRUE(cat_disp->is_valid());
        }
        else if constexpr (valuascript::shared::tuple_contains_type_v<T, AllStatementNodeTypes>)
        {
            auto stmt_disp = sample_stmt_by_kind(StatementKind::from<T>(), 0);
            ASSERT_NE(stmt_disp, nullptr);
            EXPECT_EQ(stmt_disp->kind, T::KIND);
            EXPECT_TRUE(stmt_disp->is_valid());

            auto stmt_as = sample_stmt_as<T>(0);
            ASSERT_NE(stmt_as, nullptr);
            EXPECT_EQ(stmt_as->kind, T::KIND);
            EXPECT_TRUE(stmt_as->is_valid());

            auto cat_disp = sample_node_by_kind<Statement>(T::KIND, 0);
            ASSERT_NE(cat_disp, nullptr);
            EXPECT_EQ(cat_disp->kind, T::KIND);
            EXPECT_TRUE(cat_disp->is_valid());
        }
        else if constexpr (valuascript::shared::tuple_contains_type_v<T, AllTypeAnnotationNodeTypes>)
        {
            auto type_disp = sample_type_by_kind(TypeAnnotationKind::from<T>(), 0);
            ASSERT_NE(type_disp, nullptr);
            EXPECT_EQ(type_disp->kind, T::KIND);
            EXPECT_TRUE(type_disp->is_valid());

            auto type_as = sample_type_as<T>(0);
            ASSERT_NE(type_as, nullptr);
            EXPECT_EQ(type_as->kind, T::KIND);
            EXPECT_TRUE(type_as->is_valid());

            auto cat_disp = sample_node_by_kind<TypeAnnotation>(T::KIND, 0);
            ASSERT_NE(cat_disp, nullptr);
            EXPECT_EQ(cat_disp->kind, T::KIND);
            EXPECT_TRUE(cat_disp->is_valid());
        }

        auto seq1 = create_sample<T>(0);
        auto seq2 = create_sample<T>(0);
        if constexpr (std::same_as<decltype(seq1), std::unique_ptr<T>>)
        {
            ASSERT_NE(seq1, nullptr);
            ASSERT_NE(seq2, nullptr);
            EXPECT_NE(seq1->span.start_offset, seq2->span.start_offset);
            EXPECT_NE(seq1.get(), seq2.get());
        }
        else
        {
            EXPECT_NE(seq1.span.start_offset, seq2.span.start_offset);
        }

        reset_factory_state(777);
        auto reset1 = create_sample<T>(0);
        reset_factory_state(777);
        auto reset2 = create_sample<T>(0);

        if constexpr (std::same_as<decltype(reset1), std::unique_ptr<T>>)
        {
            ASSERT_NE(reset1, nullptr);
            ASSERT_NE(reset2, nullptr);
            EXPECT_EQ(reset1->span, reset2->span);
            EXPECT_NE(reset1.get(), reset2.get());
            EXPECT_EQ(reset1->kind, T::KIND);
            EXPECT_EQ(reset2->kind, T::KIND);
            EXPECT_TRUE(reset1->is_valid());
            EXPECT_TRUE(reset2->is_valid());
        }
        else
        {
            EXPECT_EQ(reset1.span, reset2.span);
            EXPECT_EQ(reset1.kind, T::KIND);
            EXPECT_EQ(reset2.kind, T::KIND);
            EXPECT_TRUE(reset1.is_valid());
            EXPECT_TRUE(reset2.is_valid());
        }
    }

    template <typename T>
    AstFactoryTestDescriptor make_ast_factory_test_descriptor()
    {
        return AstFactoryTestDescriptor{
            .node_name = std::string(get_ast_node_name<T>()),
            .run_test = []() {
                test_single_factory_node<T>();
            }
        };
    }

    template <typename Tuple>
    struct AstFactoryTestDescriptorCollector;

    template <typename... Types>
    struct AstFactoryTestDescriptorCollector<std::tuple<Types...>>
    {
        static std::vector<AstFactoryTestDescriptor> collect()
        {
            return { make_ast_factory_test_descriptor<Types>()... };
        }
    };

    inline std::vector<AstFactoryTestDescriptor> get_all_ast_factory_test_descriptors()
    {
        return AstFactoryTestDescriptorCollector<AllAstNodeTypes>::collect();
    }

    class AstFactoryNodeParameterizedTest : public testing::TestWithParam<AstFactoryTestDescriptor>
    {
    };

    TEST_P(AstFactoryNodeParameterizedTest, NodeGenerationAndInvariants)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing AST Node Factory Generation for: " + descriptor.node_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstFactoryTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstFactoryTestDescriptor>& info) const
        {
            return info.param.node_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstFactoryNodeParameterizedTest,
        testing::ValuesIn(get_all_ast_factory_test_descriptors()),
        AstFactoryTestNameGenerator{}
    );
}
