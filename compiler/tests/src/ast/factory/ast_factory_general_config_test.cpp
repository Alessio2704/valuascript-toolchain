#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "ast/factory/ast_factory.h"
#include "ast/factory/ast_factory_config.h"
#include "ast/categories/ast_expression_types.h"
#include "ast/categories/ast_statement_types.h"
#include "ast/categories/ast_type_annotation_types.h"
#include "ast/categories/ast_reassignment_target_types.h"
#include "utils/traits/tuple_traits.h"

namespace valuascript::compiler::test
{
    TEST(AstFactoryGeneralConfigTest, DefaultValuesAreConsistent)
    {
        AstFactoryGeneralConfig general{};
        EXPECT_EQ(general.max_expression_depth, 4);
        EXPECT_EQ(general.max_type_depth, 2);
        EXPECT_EQ(general.expression_kind, AstKind::NumberLiteral);
        EXPECT_EQ(general.statement_kind, AstKind::ExpressionStatement);
        EXPECT_EQ(general.type_kind, AstKind::TypeAnnotation);
        EXPECT_EQ(general.reassignment_target_kind, AstKind::IdentifierAccess);
    }

    TEST(AstFactoryGeneralConfigTest, ExpressionDepthCutoffMatrix)
    {
        for (int max_depth : {0, 1, 2, 4})
        {
            AstFactoryConfig cfg{
                .general = {
                    .max_expression_depth = max_depth,
                    .expression_kind = ExpressionKind::from<StringLiteral>()
                }
            };

            for (int d = 0; d <= 5; ++d)
            {
                auto expr = sample_expr(d, cfg);
                ASSERT_NE(expr, nullptr);
                EXPECT_TRUE(expr->is_valid());

                if (d >= max_depth)
                {
                    EXPECT_EQ(expr->kind, AstKind::NumberLiteral);
                }
                else
                {
                    EXPECT_EQ(expr->kind, AstKind::StringLiteral);
                }
            }
        }
    }

    TEST(AstFactoryGeneralConfigTest, TypeDepthCutoffMatrix)
    {
        for (int max_depth : {0, 1, 2})
        {
            AstFactoryConfig cfg{
                .general = {
                    .max_type_depth = max_depth,
                    .type_kind = TypeAnnotationKind::from<TupleTypeAnnotation>()
                }
            };

            for (int d = 0; d <= 4; ++d)
            {
                auto type_node = sample_type(d, cfg);
                ASSERT_NE(type_node, nullptr);
                EXPECT_TRUE(type_node->is_valid());

                if (d >= max_depth || max_depth == 0)
                {
                    EXPECT_EQ(type_node->kind, AstKind::TypeAnnotation);
                    auto* leaf = dynamic_cast<TypeAnnotation*>(type_node.get());
                    ASSERT_NE(leaf, nullptr);
                    EXPECT_EQ(leaf->name.value.rfind("LeafType_", 0), 0);
                }
                else
                {
                    EXPECT_EQ(type_node->kind, AstKind::TupleTypeAnnotation);
                }
            }
        }
    }

    TEST(AstFactoryGeneralConfigTest, ExpressionKindOverridesAllExpressionTypes)
    {
        valuascript::shared::tuple_for_each_type<AllExpressionNodeTypes>([]<typename E>()
        {
            AstFactoryConfig cfg{
                .general = {
                    .max_expression_depth = 4,
                    .expression_kind = ExpressionKind::from<E>()
                }
            };

            auto expr = sample_expr(0, cfg);
            ASSERT_NE(expr, nullptr);
            EXPECT_EQ(expr->kind, E::KIND);
            EXPECT_TRUE(expr->is_valid());
        });
    }

    TEST(AstFactoryGeneralConfigTest, StatementKindOverridesAllStatementTypes)
    {
        valuascript::shared::tuple_for_each_type<AllStatementNodeTypes>([]<typename S>()
        {
            AstFactoryConfig cfg{
                .general = {
                    .statement_kind = StatementKind::from<S>()
                }
            };

            auto stmt = sample_stmt(0, cfg);
            ASSERT_NE(stmt, nullptr);
            EXPECT_EQ(stmt->kind, S::KIND);
            EXPECT_TRUE(stmt->is_valid());
        });
    }

    TEST(AstFactoryGeneralConfigTest, TypeKindOverridesAllTypeAnnotationTypes)
    {
        valuascript::shared::tuple_for_each_type<AllTypeAnnotationNodeTypes>([]<typename T>()
        {
            AstFactoryConfig cfg{
                .general = {
                    .max_type_depth = 4,
                    .type_kind = TypeAnnotationKind::from<T>()
                }
            };

            auto type_node = sample_type(0, cfg);
            ASSERT_NE(type_node, nullptr);
            EXPECT_EQ(type_node->kind, T::KIND);
            EXPECT_TRUE(type_node->is_valid());
        });
    }

    TEST(AstFactoryGeneralConfigTest, ReassignmentTargetKindOverridesAllLValueTypes)
    {
        valuascript::shared::tuple_for_each_type<AllReassignmentTargetNodeTypes>([]<typename TargetT>()
        {
            AstFactoryConfig cfg{
                .general = {
                    .reassignment_target_kind = ReassignmentTargetKind::from<TargetT>()
                }
            };

            auto reassign = create_sample<Reassignment>(0, cfg);
            ASSERT_NE(reassign, nullptr);
            EXPECT_TRUE(reassign->is_valid());
            ASSERT_NE(reassign->target, nullptr);
            EXPECT_EQ(reassign->target->kind, TargetT::KIND);
            EXPECT_TRUE(reassign->target->is_valid());
        });
    }
}
