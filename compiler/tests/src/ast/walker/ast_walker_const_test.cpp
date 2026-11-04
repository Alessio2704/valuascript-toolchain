#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <memory>

#include "ast_walker_test_helper.h"

namespace valuascript::compiler::test
{
    template <typename NodeT>
    class ConstCountingWalker : public ConstAstWalker
    {
    public:
        using ConstAstWalker::enter;
        using ConstAstWalker::leave;

        size_t count = 0;
        bool entered_concrete = false;

        TraversalAction enter(const NodeT&) override
        {
            entered_concrete = true;
            return TraversalAction::Continue;
        }

        TraversalAction enter_node(const AstNode&) override
        {
            count++;
            return TraversalAction::Continue;
        }
    };

    template <typename NodeT>
    class SpanMutatingWalker : public AstWalker
    {
    public:
        using AstWalker::enter;
        using AstWalker::leave;

        TraversalAction enter_node(AstNode& node) override
        {
            node.span.length = 999;
            return TraversalAction::Continue;
        }
    };

    template <typename NodeT>
    inline void run_const_and_mutating_test_for_node()
    {
        {
            auto sample = create_sample<NodeT>(2);
            ConstCountingWalker<NodeT> const_walker;

            if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
            {
                const auto& const_ref = sample;
                const_walker.walk(const_ref);
            }
            else
            {
                ASSERT_NE(sample, nullptr);
                const auto* const_ptr = sample.get();
                const_walker.walk(const_ptr);
            }

            EXPECT_TRUE(const_walker.entered_concrete);
            EXPECT_GT(const_walker.count, 0);
        }

        {
            auto sample = create_sample<NodeT>(2);
            SpanMutatingWalker<NodeT> mut_walker;

            if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
            {
                mut_walker.walk(sample);
                EXPECT_EQ(sample.span.length, 999);
            }
            else
            {
                ASSERT_NE(sample, nullptr);
                mut_walker.walk(sample.get());
                EXPECT_EQ(sample->span.length, 999);
            }
        }
    }

    template <typename NodeT>
    struct AstWalkerConstTestBuilder
    {
        static AstWalkerTestDescriptor build()
        {
            return AstWalkerTestDescriptor{
                .name = std::string(get_ast_node_name<NodeT>()),
                .run_test = []()
                {
                    run_const_and_mutating_test_for_node<NodeT>();
                }
            };
        }
    };

    class AstWalkerConstParameterizedTest : public testing::TestWithParam<AstWalkerTestDescriptor>
    {
    };

    TEST_P(AstWalkerConstParameterizedTest, SupportsConstAndMutableTraversals)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Const and Mutating Walker for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstWalkerConstParameterizedTest,
        testing::ValuesIn((AstWalkerDescriptorCollector<AllAstNodeTypes, AstWalkerConstTestBuilder>::collect())),
        AstWalkerTestNameGenerator{}
    );
}
