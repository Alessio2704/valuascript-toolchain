#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <memory>

#include "ast_walker_test_helper.h"

namespace valuascript::compiler::test
{
    TEST(AstWalkerBoundaryTest, WalkNullptrIsSafeNoOp)
    {
        AstWalker walker;
        AstNode* null_node = nullptr;
        walker.walk(null_node);

        EXPECT_EQ(walker.depth(), 0);
        EXPECT_EQ(walker.parent(), nullptr);
        EXPECT_TRUE(walker.is_root());
        EXPECT_TRUE(walker.ancestor_stack().empty());
        EXPECT_EQ(walker.find_ancestor<Program>(), nullptr);
    }

    TEST(AstWalkerBoundaryTest, PreWalkEmptyStackQueriesReturnExpectedDefaults)
    {
        ConstAstWalker const_walker;
        EXPECT_EQ(const_walker.depth(), 0);
        EXPECT_EQ(const_walker.parent(), nullptr);
        EXPECT_TRUE(const_walker.is_root());
        EXPECT_TRUE(const_walker.ancestor_stack().empty());
        EXPECT_EQ(const_walker.find_ancestor<FunctionDefinition>(), nullptr);

        AstWalker mut_walker;
        EXPECT_EQ(mut_walker.depth(), 0);
        EXPECT_EQ(mut_walker.parent(), nullptr);
        EXPECT_TRUE(mut_walker.is_root());
        EXPECT_TRUE(mut_walker.ancestor_stack().empty());
        EXPECT_EQ(mut_walker.find_ancestor<FunctionDefinition>(), nullptr);
    }

    template <typename NodeT>
    class NegativeAncestorTrackingWalker : public AstWalker
    {
    public:
        using AstWalker::enter;
        using AstWalker::leave;

        bool verified_negative_lookup = false;

        TraversalAction enter_node(AstNode&) override
        {
            if constexpr (std::same_as<NodeT, Program>)
            {
                if (depth() == 1)
                {
                    EXPECT_EQ(find_ancestor<PercentageLiteral>(), nullptr);
                }
            }
            else
            {
                EXPECT_EQ(find_ancestor<Program>(), nullptr);
            }
            verified_negative_lookup = true;
            return TraversalAction::Continue;
        }
    };

    template <typename NodeT>
    inline void run_negative_ancestor_test_for_node()
    {
        auto sample = create_sample<NodeT>(1);
        NegativeAncestorTrackingWalker<NodeT> walker;

        if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
        {
            walker.walk(sample);
        }
        else
        {
            ASSERT_NE(sample, nullptr);
            walker.walk(sample.get());
        }

        EXPECT_TRUE(walker.verified_negative_lookup);
    }

    template <typename NodeT>
    struct AstWalkerNegativeAncestorTestBuilder
    {
        static AstWalkerTestDescriptor build()
        {
            return AstWalkerTestDescriptor{
                .name = std::string(get_ast_node_name<NodeT>()),
                .run_test = []()
                {
                    run_negative_ancestor_test_for_node<NodeT>();
                }
            };
        }
    };

    class AstWalkerNegativeAncestorParameterizedTest : public testing::TestWithParam<AstWalkerTestDescriptor>
    {
    };

    TEST_P(AstWalkerNegativeAncestorParameterizedTest, ReturnsNullptrForAbsentAncestors)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Negative Ancestor Lookup for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstWalkerNegativeAncestorParameterizedTest,
        testing::ValuesIn((AstWalkerDescriptorCollector<AllAstNodeTypes, AstWalkerNegativeAncestorTestBuilder>::collect())),
        AstWalkerTestNameGenerator{}
    );

    class EnterNodeStoppingWalker : public AstWalker
    {
    public:
        using AstWalker::enter;
        using AstWalker::leave;

        size_t total_enter_nodes = 0;
        size_t total_leave_nodes = 0;

        TraversalAction enter_node(AstNode&) override
        {
            total_enter_nodes++;
            return TraversalAction::Stop;
        }

        void leave_node(AstNode&) override
        {
            total_leave_nodes++;
        }
    };

    TEST(AstWalkerBoundaryTest, StopFromEnterNodeHaltsImmediately)
    {
        auto prog = create_sample_program(2);
        EnterNodeStoppingWalker walker;
        walker.walk(*prog);

        EXPECT_EQ(walker.total_enter_nodes, 1);
        EXPECT_EQ(walker.total_leave_nodes, 0);
        EXPECT_EQ(walker.depth(), 0);
    }

    class CategoryStoppingWalker : public AstWalker
    {
    public:
        using AstWalker::enter;
        using AstWalker::leave;

        size_t entered_expressions = 0;
        size_t left_expressions = 0;

        TraversalAction enter(Expression&) override
        {
            entered_expressions++;
            return TraversalAction::Stop;
        }

        void leave(Expression&) override
        {
            left_expressions++;
        }
    };

    TEST(AstWalkerBoundaryTest, StopFromCategoryHookHaltsImmediately)
    {
        auto binary = create_sample<BinaryExpression>(2);
        ASSERT_NE(binary, nullptr);

        CategoryStoppingWalker walker;
        walker.walk(binary.get());

        EXPECT_EQ(walker.entered_expressions, 1);
        EXPECT_EQ(walker.left_expressions, 0);
        EXPECT_EQ(walker.depth(), 0);
    }

    class CountingOrStoppingWalker : public AstWalker
    {
    public:
        using AstWalker::enter;
        using AstWalker::leave;

        bool should_stop_on_first = true;
        size_t total_visits = 0;

        TraversalAction enter_node(AstNode&) override
        {
            total_visits++;
            if (should_stop_on_first)
            {
                return TraversalAction::Stop;
            }
            return TraversalAction::Continue;
        }
    };

    TEST(AstWalkerBoundaryTest, WalkerInstanceIsReusableAcrossMultipleWalks)
    {
        auto prog1 = create_sample_program(1);
        auto prog2 = create_sample_program(1);

        CountingOrStoppingWalker walker;
        walker.should_stop_on_first = true;
        walker.walk(*prog1);

        EXPECT_EQ(walker.total_visits, 1);
        EXPECT_EQ(walker.depth(), 0);

        walker.should_stop_on_first = false;
        walker.total_visits = 0;
        walker.walk(*prog2);

        EXPECT_GT(walker.total_visits, 1);
        EXPECT_EQ(walker.depth(), 0);
    }
}
