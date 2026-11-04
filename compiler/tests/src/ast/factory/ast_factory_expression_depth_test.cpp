#include <gtest/gtest.h>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <array>
#include <type_traits>
#include <utility>

#include "ast/factory/ast_factory.h"
#include "ast/factory/ast_factory_config.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/categories/ast_expression_types.h"

namespace valuascript::compiler::test
{
    template <typename Visitor>
    void visit_child_expressions(const auto& node, Visitor&& visitor)
    {
        for_each_ast_member(node, [&](const auto& member)
        {
            using MemberType = std::remove_cvref_t<decltype(member)>;
            if constexpr (std::same_as<MemberType, ExprPtr>)
            {
                if (member != nullptr)
                {
                    visitor(*member);
                }
            }
            else if constexpr (std::same_as<MemberType, OptionalAstField<ExprPtr>>)
            {
                if (member.has_value() && member.get() != nullptr)
                {
                    visitor(*member);
                }
            }
            else if constexpr (std::same_as<MemberType, std::vector<ExprPtr>>)
            {
                for (const auto& elem : member)
                {
                    if (elem != nullptr)
                    {
                        visitor(*elem);
                    }
                }
            }
            else if constexpr (std::same_as<MemberType, std::vector<CallArgument>>)
            {
                for (const auto& arg : member)
                {
                    if (arg.value != nullptr)
                    {
                        visitor(*arg.value);
                    }
                }
            }
            else if constexpr (std::same_as<MemberType, std::vector<DictItem>>)
            {
                for (const auto& item : member)
                {
                    if (item.value != nullptr)
                    {
                        visitor(*item.value);
                    }
                }
            }
            else if constexpr (std::same_as<MemberType, std::vector<SwitchCase>>)
            {
                for (const auto& sc : member)
                {
                    if (sc.result != nullptr)
                    {
                        visitor(*sc.result);
                    }
                }
            }
        });
    }

    using DynamicExpressionContainers = std::tuple<
        BinaryExpression,
        UnaryExpression,
        GroupingExpression,
        ConditionalExpression,
        FunctionCall,
        DictLiteral,
        TensorLiteral,
        TupleLiteral,
        BracketAccess,
        DotAccess,
        SwitchExpression
    >;

    template <typename ContainerT, typename ElemExprT>
    void test_container_nests_expression()
    {
        AstFactoryConfig cfg{
            .general = {
                .max_expression_depth = 4,
                .expression_kind = ExpressionKind::from<ElemExprT>()
            }
        };

        auto container = create_sample<ContainerT>(0, cfg);
        ASSERT_NE(container, nullptr);
        EXPECT_TRUE(container->is_valid());

        bool found_child = false;
        visit_child_expressions(*container, [&](const Expression& child)
        {
            found_child = true;
            EXPECT_EQ(child.kind, ElemExprT::KIND);
            EXPECT_TRUE(child.is_valid());
            EXPECT_GE(child.span.column_start, 5);
        });

        EXPECT_TRUE(found_child);
    }

    template <typename ElemExprT, typename ContainerTuple>
    struct AllContainersNestingTester;

    template <typename ElemExprT, typename... Containers>
    struct AllContainersNestingTester<ElemExprT, std::tuple<Containers...>>
    {
        static void run()
        {
            (test_container_nests_expression<Containers, ElemExprT>(), ...);
        }
    };

    struct AstExpressionDepthTestDescriptor
    {
        std::string expression_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstExpressionDepthTestDescriptor& desc)
        {
            return os << desc.expression_name;
        }
    };

    template <typename E>
    inline void test_single_expression_depth_handling()
    {
        constexpr std::array<int, 5> depths{0, 1, 2, 3, 4};
        for (int depth : depths)
        {
            auto expr = create_sample<E>(depth);
            ASSERT_NE(expr, nullptr);
            EXPECT_EQ(expr->kind, E::KIND);
            EXPECT_TRUE(expr->is_valid());

            const size_t expected_col_start = (static_cast<size_t>(depth) * 4) + 1;
            const size_t expected_col_end = (static_cast<size_t>(depth) * 4) + 13;
            EXPECT_EQ(expr->span.column_start, expected_col_start);
            EXPECT_EQ(expr->span.column_end, expected_col_end);

            visit_child_expressions(*expr, [&](const Expression& child)
            {
                EXPECT_GE(child.span.column_start, ((static_cast<size_t>(depth) + 1) * 4) + 1);
                EXPECT_TRUE(child.is_valid());
            });
        }

        AllContainersNestingTester<E, DynamicExpressionContainers>::run();

        AstFactoryConfig cutoff_cfg{
            .general = {
                .max_expression_depth = 2,
                .expression_kind = ExpressionKind::from<E>()
            }
        };

        auto at_0 = sample_expr(0, cutoff_cfg);
        ASSERT_NE(at_0, nullptr);
        EXPECT_EQ(at_0->kind, E::KIND);

        auto at_1 = sample_expr(1, cutoff_cfg);
        ASSERT_NE(at_1, nullptr);
        EXPECT_EQ(at_1->kind, E::KIND);

        auto at_2 = sample_expr(2, cutoff_cfg);
        ASSERT_NE(at_2, nullptr);
        EXPECT_EQ(at_2->kind, AstKind::NumberLiteral);

        auto at_3 = sample_expr(3, cutoff_cfg);
        ASSERT_NE(at_3, nullptr);
        EXPECT_EQ(at_3->kind, AstKind::NumberLiteral);
    }

    template <typename E>
    AstExpressionDepthTestDescriptor make_ast_expression_depth_test_descriptor()
    {
        return AstExpressionDepthTestDescriptor{
            .expression_name = std::string(get_ast_node_name<E>()),
            .run_test = []() {
                test_single_expression_depth_handling<E>();
            }
        };
    }

    template <typename Tuple>
    struct AstExpressionDepthTestDescriptorCollector;

    template <typename... ExprTypes>
    struct AstExpressionDepthTestDescriptorCollector<std::tuple<ExprTypes...>>
    {
        static std::vector<AstExpressionDepthTestDescriptor> collect()
        {
            return { make_ast_expression_depth_test_descriptor<ExprTypes>()... };
        }
    };

    inline std::vector<AstExpressionDepthTestDescriptor> get_all_ast_expression_depth_test_descriptors()
    {
        return AstExpressionDepthTestDescriptorCollector<AllExpressionNodeTypes>::collect();
    }

    class AstFactoryExpressionDepthParameterizedTest : public testing::TestWithParam<AstExpressionDepthTestDescriptor>
    {
    };

    TEST_P(AstFactoryExpressionDepthParameterizedTest, ExpressionDepthAndNestingInvariants)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Expression Depth & Nesting for: " + descriptor.expression_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstExpressionTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstExpressionDepthTestDescriptor>& info) const
        {
            return info.param.expression_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllExpressionDepthTests,
        AstFactoryExpressionDepthParameterizedTest,
        testing::ValuesIn(get_all_ast_expression_depth_test_descriptors()),
        AstExpressionTestNameGenerator{}
    );
}
