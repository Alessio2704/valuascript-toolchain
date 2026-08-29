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
#include "ast/categories/ast_statement_types.h"

namespace valuascript::compiler::test
{
    template <typename Visitor>
    void visit_statement_child_expressions(const auto& stmt, Visitor&& visitor)
    {
        for_each_ast_member(stmt, [&](const auto& member)
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
        });
    }

    struct AstStatementDepthTestDescriptor
    {
        std::string statement_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstStatementDepthTestDescriptor& desc)
        {
            return os << desc.statement_name;
        }
    };

    template <typename S>
    inline void test_single_statement_depth_handling()
    {
        constexpr std::array<int, 5> depths{0, 1, 2, 3, 4};
        for (int depth : depths)
        {
            auto stmt = create_sample<S>(depth);
            ASSERT_NE(stmt, nullptr);
            EXPECT_EQ(stmt->kind, S::KIND);
            EXPECT_TRUE(stmt->is_valid());

            const size_t expected_col_start = (static_cast<size_t>(depth) * 4) + 1;
            const size_t expected_col_end = (static_cast<size_t>(depth) * 4) + 13;
            EXPECT_EQ(stmt->span.column_start, expected_col_start);
            EXPECT_EQ(stmt->span.column_end, expected_col_end);

            visit_statement_child_expressions(*stmt, [&](const Expression& child)
            {
                EXPECT_GE(child.span.column_start, ((static_cast<size_t>(depth) + 1) * 4) + 1);
                EXPECT_TRUE(child.is_valid());
            });
        }

        AstFactoryConfig cfg{
            .general = {
                .max_expression_depth = 4,
                .expression_kind = ExpressionKind::from<StringLiteral>(),
                .statement_kind = StatementKind::from<S>(),
                .reassignment_target_kind = ReassignmentTargetKind::from<IdentifierAccess>()
            }
        };

        auto sampled_dynamic = sample_stmt(0, cfg);
        ASSERT_NE(sampled_dynamic, nullptr);
        EXPECT_EQ(sampled_dynamic->kind, S::KIND);
        EXPECT_TRUE(sampled_dynamic->is_valid());

        auto typed_sampled = create_sample<S>(0, cfg);
        ASSERT_NE(typed_sampled, nullptr);
        EXPECT_EQ(typed_sampled->kind, S::KIND);
        EXPECT_TRUE(typed_sampled->is_valid());

        if constexpr (std::same_as<S, Assignment>)
        {
            ASSERT_NE(typed_sampled->value, nullptr);
            EXPECT_EQ(typed_sampled->value->kind, AstKind::StringLiteral);
            EXPECT_TRUE(typed_sampled->value->is_valid());
            EXPECT_GE(typed_sampled->value->span.column_start, 5);
        }
        else if constexpr (std::same_as<S, Reassignment>)
        {
            ASSERT_NE(typed_sampled->target, nullptr);
            EXPECT_EQ(typed_sampled->target->kind, AstKind::IdentifierAccess);
            EXPECT_TRUE(typed_sampled->target->is_valid());
            EXPECT_GE(typed_sampled->target->span.column_start, 5);

            ASSERT_NE(typed_sampled->value, nullptr);
            EXPECT_EQ(typed_sampled->value->kind, AstKind::StringLiteral);
            EXPECT_TRUE(typed_sampled->value->is_valid());
            EXPECT_GE(typed_sampled->value->span.column_start, 5);
        }
        else if constexpr (std::same_as<S, ExpressionStatement>)
        {
            ASSERT_NE(typed_sampled->expr, nullptr);
            EXPECT_EQ(typed_sampled->expr->kind, AstKind::StringLiteral);
            EXPECT_TRUE(typed_sampled->expr->is_valid());
            EXPECT_GE(typed_sampled->expr->span.column_start, 5);
        }
        else if constexpr (std::same_as<S, ReturnStatement>)
        {
            for (const auto& ret_val : typed_sampled->values)
            {
                if (ret_val != nullptr)
                {
                    EXPECT_EQ(ret_val->kind, AstKind::StringLiteral);
                    EXPECT_TRUE(ret_val->is_valid());
                    EXPECT_GE(ret_val->span.column_start, 5);
                }
            }
        }
    }

    template <typename S>
    AstStatementDepthTestDescriptor make_ast_statement_depth_test_descriptor()
    {
        return AstStatementDepthTestDescriptor{
            .statement_name = std::string(get_ast_node_name<S>()),
            .run_test = []() {
                test_single_statement_depth_handling<S>();
            }
        };
    }

    template <typename Tuple>
    struct AstStatementDepthTestDescriptorCollector;

    template <typename... StmtTypes>
    struct AstStatementDepthTestDescriptorCollector<std::tuple<StmtTypes...>>
    {
        static std::vector<AstStatementDepthTestDescriptor> collect()
        {
            return { make_ast_statement_depth_test_descriptor<StmtTypes>()... };
        }
    };

    inline std::vector<AstStatementDepthTestDescriptor> get_all_ast_statement_depth_test_descriptors()
    {
        return AstStatementDepthTestDescriptorCollector<AllStatementNodeTypes>::collect();
    }

    class AstFactoryStatementDepthParameterizedTest : public testing::TestWithParam<AstStatementDepthTestDescriptor>
    {
    };

    TEST_P(AstFactoryStatementDepthParameterizedTest, StatementDepthAndExpressionNestingInvariants)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Statement Depth & Expression Nesting for: " + descriptor.statement_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstStatementTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstStatementDepthTestDescriptor>& info) const
        {
            return info.param.statement_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllStatementDepthTests,
        AstFactoryStatementDepthParameterizedTest,
        testing::ValuesIn(get_all_ast_statement_depth_test_descriptors()),
        AstStatementTestNameGenerator{}
    );
}
