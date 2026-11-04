#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <utility>

#include "ast_rewriter_test_helper.h"

namespace valuascript::compiler::test
{
    template <typename TargetStmtT>
    class GenericStatementSubstituter : public AstRewriter
    {
    public:
        using AstRewriter::rewrite;

        StmtPtr rewrite(StmtPtr stmt) override
        {
            if (!stmt) return nullptr;
            return create_sample<TargetStmtT>(0);
        }
    };

    template <typename TargetExprT>
    class GenericExpressionSubstituter : public AstRewriter
    {
    public:
        using AstRewriter::rewrite;

        ExprPtr rewrite(ExprPtr expr) override
        {
            if (!expr) return nullptr;
            return create_sample<TargetExprT>(0);
        }
    };

    template <typename TargetTypeT>
    class GenericTypeAnnotationSubstituter : public AstRewriter
    {
    public:
        using AstRewriter::rewrite;

        TypeAnnPtr rewrite(TypeAnnPtr type) override
        {
            if (!type) return nullptr;
            return create_sample<TargetTypeT>(0);
        }
    };

    using FirstStatementType = std::tuple_element_t<0, AllStatementNodeTypes>;
    using FirstExpressionType = std::tuple_element_t<0, AllExpressionNodeTypes>;
    using FirstTypeAnnotationType = std::tuple_element_t<0, AllTypeAnnotationNodeTypes>;

    template <typename StmtT>
    inline void run_statement_polymorphism_test()
    {
        AstRewriter default_rewriter;
        StmtPtr stmt = create_sample<StmtT>(2);
        ASSERT_NE(stmt, nullptr);

        auto cloned = clone_node(stmt);
        auto rewritten = default_rewriter.rewrite(std::move(cloned));
        ASSERT_NE(rewritten, nullptr);
        EXPECT_EQ(rewritten->kind, stmt->kind);
        EXPECT_TRUE(ast_equals(rewritten, stmt));

        GenericStatementSubstituter<FirstStatementType> substituter;
        auto replaced = substituter.rewrite(std::move(stmt));
        ASSERT_NE(replaced, nullptr);
        EXPECT_EQ(replaced->kind, FirstStatementType::KIND);
    }

    template <typename ExprT>
    inline void run_expression_polymorphism_test()
    {
        AstRewriter default_rewriter;
        ExprPtr expr = create_sample<ExprT>(2);
        ASSERT_NE(expr, nullptr);

        auto cloned = clone_node(expr);
        auto rewritten = default_rewriter.rewrite(std::move(cloned));
        ASSERT_NE(rewritten, nullptr);
        EXPECT_EQ(rewritten->kind, expr->kind);
        EXPECT_TRUE(ast_equals(rewritten, expr));

        GenericExpressionSubstituter<FirstExpressionType> substituter;
        auto replaced = substituter.rewrite(std::move(expr));
        ASSERT_NE(replaced, nullptr);
        EXPECT_EQ(replaced->kind, FirstExpressionType::KIND);
    }

    template <typename TypeT>
    inline void run_type_annotation_polymorphism_test()
    {
        AstRewriter default_rewriter;
        TypeAnnPtr type = create_sample<TypeT>(2);
        ASSERT_NE(type, nullptr);

        auto cloned = clone_node(type);
        auto rewritten = default_rewriter.rewrite(std::move(cloned));
        ASSERT_NE(rewritten, nullptr);
        EXPECT_EQ(rewritten->kind, type->kind);
        EXPECT_TRUE(ast_equals(rewritten, type));

        GenericTypeAnnotationSubstituter<FirstTypeAnnotationType> substituter;
        auto replaced = substituter.rewrite(std::move(type));
        ASSERT_NE(replaced, nullptr);
        EXPECT_EQ(replaced->kind, FirstTypeAnnotationType::KIND);
    }

    template <typename StmtT>
    struct StatementPolymorphismTestBuilder
    {
        static AstRewriterTestDescriptor build()
        {
            return AstRewriterTestDescriptor{
                .name = std::string(get_ast_node_name<StmtT>()),
                .run_test = []()
                {
                    run_statement_polymorphism_test<StmtT>();
                }
            };
        }
    };

    template <typename ExprT>
    struct ExpressionPolymorphismTestBuilder
    {
        static AstRewriterTestDescriptor build()
        {
            return AstRewriterTestDescriptor{
                .name = std::string(get_ast_node_name<ExprT>()),
                .run_test = []()
                {
                    run_expression_polymorphism_test<ExprT>();
                }
            };
        }
    };

    template <typename TypeT>
    struct TypeAnnotationPolymorphismTestBuilder
    {
        static AstRewriterTestDescriptor build()
        {
            return AstRewriterTestDescriptor{
                .name = std::string(get_ast_node_name<TypeT>()),
                .run_test = []()
                {
                    run_type_annotation_polymorphism_test<TypeT>();
                }
            };
        }
    };

    class StatementPolymorphismParameterizedTest : public testing::TestWithParam<AstRewriterTestDescriptor>
    {
    };

    class ExpressionPolymorphismParameterizedTest : public testing::TestWithParam<AstRewriterTestDescriptor>
    {
    };

    class TypeAnnotationPolymorphismParameterizedTest : public testing::TestWithParam<AstRewriterTestDescriptor>
    {
    };

    TEST_P(StatementPolymorphismParameterizedTest, DispatchesAndReplacesPolymorphically)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Statement Category Polymorphism for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    TEST_P(ExpressionPolymorphismParameterizedTest, DispatchesAndReplacesPolymorphically)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Expression Category Polymorphism for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    TEST_P(TypeAnnotationPolymorphismParameterizedTest, DispatchesAndReplacesPolymorphically)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Type Annotation Category Polymorphism for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllStatements,
        StatementPolymorphismParameterizedTest,
        testing::ValuesIn((AstRewriterDescriptorCollector<AllStatementNodeTypes, StatementPolymorphismTestBuilder>::collect())),
        AstRewriterTestNameGenerator{}
    );

    INSTANTIATE_TEST_SUITE_P(
        AllExpressions,
        ExpressionPolymorphismParameterizedTest,
        testing::ValuesIn((AstRewriterDescriptorCollector<AllExpressionNodeTypes, ExpressionPolymorphismTestBuilder>::collect())),
        AstRewriterTestNameGenerator{}
    );

    INSTANTIATE_TEST_SUITE_P(
        AllTypeAnnotations,
        TypeAnnotationPolymorphismParameterizedTest,
        testing::ValuesIn((AstRewriterDescriptorCollector<AllTypeAnnotationNodeTypes, TypeAnnotationPolymorphismTestBuilder>::collect())),
        AstRewriterTestNameGenerator{}
    );
}
