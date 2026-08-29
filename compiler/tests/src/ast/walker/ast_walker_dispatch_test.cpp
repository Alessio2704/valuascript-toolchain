#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <memory>

#include "ast_walker_test_helper.h"

namespace valuascript::compiler::test
{
    template <typename NodeT>
    class ConcreteDispatchWalker : public AstWalker
    {
    public:
        using AstWalker::enter;
        using AstWalker::leave;

        bool entered_concrete = false;
        bool left_concrete = false;
        size_t enter_node_count = 0;
        size_t leave_node_count = 0;

        TraversalAction enter(NodeT&) override
        {
            entered_concrete = true;
            return TraversalAction::Continue;
        }

        void leave(NodeT&) override
        {
            left_concrete = true;
        }

        TraversalAction enter_node(AstNode&) override
        {
            enter_node_count++;
            return TraversalAction::Continue;
        }

        void leave_node(AstNode&) override
        {
            leave_node_count++;
        }
    };

    class CategoryTrackingWalker : public AstWalker
    {
    public:
        using AstWalker::enter;
        using AstWalker::leave;

        size_t entered_statement = 0;
        size_t left_statement = 0;
        size_t entered_expression = 0;
        size_t left_expression = 0;
        size_t entered_type_annotation = 0;
        size_t left_type_annotation = 0;

        TraversalAction enter(Statement&) override
        {
            entered_statement++;
            return TraversalAction::Continue;
        }

        void leave(Statement&) override
        {
            left_statement++;
        }

        TraversalAction enter(Expression&) override
        {
            entered_expression++;
            return TraversalAction::Continue;
        }

        void leave(Expression&) override
        {
            left_expression++;
        }

        TraversalAction enter(TypeAnnotation&) override
        {
            entered_type_annotation++;
            return TraversalAction::Continue;
        }

        void leave(TypeAnnotation&) override
        {
            left_type_annotation++;
        }
    };

    template <typename NodeT>
    inline void run_dispatch_test_for_node()
    {
        {
            auto sample = create_sample<NodeT>(2);
            ConcreteDispatchWalker<NodeT> walker;

            if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
            {
                walker.walk(sample);
            }
            else
            {
                ASSERT_NE(sample, nullptr);
                walker.walk(sample.get());
            }

            EXPECT_TRUE(walker.entered_concrete);
            EXPECT_TRUE(walker.left_concrete);
            EXPECT_GT(walker.enter_node_count, 0);
            EXPECT_EQ(walker.enter_node_count, walker.leave_node_count);
        }

        {
            auto sample = create_sample<NodeT>(2);
            CategoryTrackingWalker cat_walker;

            if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
            {
                cat_walker.walk(sample);
            }
            else
            {
                ASSERT_NE(sample, nullptr);
                cat_walker.walk(sample.get());
            }

            if constexpr (IsExpressionNode<NodeT>)
            {
                EXPECT_GT(cat_walker.entered_expression, 0);
                EXPECT_EQ(cat_walker.entered_expression, cat_walker.left_expression);
            }
            else
            {
                EXPECT_EQ(cat_walker.entered_expression, cat_walker.left_expression);
            }

            if constexpr (IsStatementNode<NodeT>)
            {
                EXPECT_GT(cat_walker.entered_statement, 0);
                EXPECT_EQ(cat_walker.entered_statement, cat_walker.left_statement);
            }
            else
            {
                EXPECT_EQ(cat_walker.entered_statement, cat_walker.left_statement);
            }

            if constexpr (IsTypeAnnotationNode<NodeT>)
            {
                EXPECT_GT(cat_walker.entered_type_annotation, 0);
                EXPECT_EQ(cat_walker.entered_type_annotation, cat_walker.left_type_annotation);
            }
            else
            {
                EXPECT_EQ(cat_walker.entered_type_annotation, cat_walker.left_type_annotation);
            }
        }
    }

    template <typename NodeT>
    struct AstWalkerDispatchTestBuilder
    {
        static AstWalkerTestDescriptor build()
        {
            return AstWalkerTestDescriptor{
                .name = std::string(get_ast_node_name<NodeT>()),
                .run_test = []()
                {
                    run_dispatch_test_for_node<NodeT>();
                }
            };
        }
    };

    class AstWalkerDispatchParameterizedTest : public testing::TestWithParam<AstWalkerTestDescriptor>
    {
    };

    TEST_P(AstWalkerDispatchParameterizedTest, VisitsConcreteAndCategoryHooksCorrectly)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Walker Dispatch for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstWalkerDispatchParameterizedTest,
        testing::ValuesIn((AstWalkerDescriptorCollector<AllAstNodeTypes, AstWalkerDispatchTestBuilder>::collect())),
        AstWalkerTestNameGenerator{}
    );
}
