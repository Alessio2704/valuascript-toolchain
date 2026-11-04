#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <memory>

#include "ast_walker_test_helper.h"

namespace valuascript::compiler::test
{
    template <typename NodeT>
    class SkipChildrenWalker : public AstWalker
    {
    public:
        using AstWalker::enter;
        using AstWalker::leave;

        bool enter_called = false;
        bool leave_called = false;
        size_t total_enter_nodes = 0;
        size_t total_leave_nodes = 0;

        TraversalAction enter(NodeT&) override
        {
            enter_called = true;
            return TraversalAction::SkipChildren;
        }

        void leave(NodeT&) override
        {
            leave_called = true;
        }

        TraversalAction enter_node(AstNode&) override
        {
            total_enter_nodes++;
            return TraversalAction::Continue;
        }

        void leave_node(AstNode&) override
        {
            total_leave_nodes++;
        }
    };

    template <typename NodeT>
    class StopAtRootWalker : public AstWalker
    {
    public:
        using AstWalker::enter;
        using AstWalker::leave;

        bool enter_called = false;
        bool leave_called = false;
        size_t total_enter_nodes = 0;
        size_t total_leave_nodes = 0;

        TraversalAction enter(NodeT&) override
        {
            enter_called = true;
            return TraversalAction::Stop;
        }

        void leave(NodeT&) override
        {
            leave_called = true;
        }

        TraversalAction enter_node(AstNode&) override
        {
            total_enter_nodes++;
            return TraversalAction::Continue;
        }

        void leave_node(AstNode&) override
        {
            total_leave_nodes++;
        }
    };

    template <typename NodeT>
    inline void run_control_flow_test_for_node()
    {
        {
            auto sample = create_sample<NodeT>(2);
            SkipChildrenWalker<NodeT> skip_walker;

            if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
            {
                skip_walker.walk(sample);
            }
            else
            {
                ASSERT_NE(sample, nullptr);
                skip_walker.walk(sample.get());
            }

            EXPECT_TRUE(skip_walker.enter_called);
            EXPECT_TRUE(skip_walker.leave_called);
            EXPECT_EQ(skip_walker.total_enter_nodes, 1);
            EXPECT_EQ(skip_walker.total_leave_nodes, 1);
        }

        {
            auto sample = create_sample<NodeT>(2);
            StopAtRootWalker<NodeT> stop_walker;

            if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
            {
                stop_walker.walk(sample);
            }
            else
            {
                ASSERT_NE(sample, nullptr);
                stop_walker.walk(sample.get());
            }

            EXPECT_TRUE(stop_walker.enter_called);
            EXPECT_FALSE(stop_walker.leave_called);
            EXPECT_EQ(stop_walker.total_enter_nodes, 1);
            EXPECT_EQ(stop_walker.total_leave_nodes, 0);
        }
    }

    template <typename NodeT>
    struct AstWalkerControlFlowTestBuilder
    {
        static AstWalkerTestDescriptor build()
        {
            return AstWalkerTestDescriptor{
                .name = std::string(get_ast_node_name<NodeT>()),
                .run_test = []()
                {
                    run_control_flow_test_for_node<NodeT>();
                }
            };
        }
    };

    class AstWalkerControlFlowParameterizedTest : public testing::TestWithParam<AstWalkerTestDescriptor>
    {
    };

    TEST_P(AstWalkerControlFlowParameterizedTest, ObeysTraversalActionDirectives)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Walker Control Flow for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstWalkerControlFlowParameterizedTest,
        testing::ValuesIn((AstWalkerDescriptorCollector<AllAstNodeTypes, AstWalkerControlFlowTestBuilder>::collect())),
        AstWalkerTestNameGenerator{}
    );
}
