#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <memory>

#include "ast_walker_test_helper.h"

namespace valuascript::compiler::test
{
    template <typename RootNodeT>
    class AncestryTrackingWalker : public AstWalker
    {
    public:
        size_t node_count = 0;
        size_t max_recorded_depth = 0;
        bool root_checked = false;
        bool ancestor_found_on_nested = false;

        TraversalAction enter_node(AstNode& node) override
        {
            node_count++;
            size_t current_depth = depth();
            max_recorded_depth = std::max(max_recorded_depth, current_depth);

            EXPECT_EQ(current_depth, ancestor_stack().size());
            EXPECT_GT(current_depth, 0);
            EXPECT_EQ(ancestor_stack().back(), &node);

            if (current_depth == 1)
            {
                EXPECT_TRUE(is_root());
                EXPECT_EQ(parent(), nullptr);
                root_checked = true;
            }
            else
            {
                EXPECT_FALSE(is_root());
                EXPECT_NE(parent(), nullptr);
                if (parent() != nullptr)
                {
                    EXPECT_EQ(parent(), ancestor_stack()[current_depth - 2]);
                }

                if (auto* ancestor = find_ancestor<RootNodeT>())
                {
                    EXPECT_EQ(ancestor, ancestor_stack().front());
                    ancestor_found_on_nested = true;
                }
            }

            return TraversalAction::Continue;
        }

        void leave_node(AstNode& node) override
        {
            EXPECT_EQ(depth(), ancestor_stack().size());
            EXPECT_EQ(ancestor_stack().back(), &node);
        }
    };

    template <typename NodeT>
    inline void run_ancestry_test_for_node()
    {
        auto sample = create_sample<NodeT>(2);
        AncestryTrackingWalker<NodeT> walker;

        if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
        {
            walker.walk(sample);
        }
        else
        {
            ASSERT_NE(sample, nullptr);
            walker.walk(sample.get());
        }

        EXPECT_TRUE(walker.root_checked);
        EXPECT_GT(walker.node_count, 0);
        EXPECT_GT(walker.max_recorded_depth, 0);
    }

    template <typename NodeT>
    struct AstWalkerAncestryTestBuilder
    {
        static AstWalkerTestDescriptor build()
        {
            return AstWalkerTestDescriptor{
                .name = std::string(get_ast_node_name<NodeT>()),
                .run_test = []()
                {
                    run_ancestry_test_for_node<NodeT>();
                }
            };
        }
    };

    class AstWalkerAncestryParameterizedTest : public testing::TestWithParam<AstWalkerTestDescriptor>
    {
    };

    TEST_P(AstWalkerAncestryParameterizedTest, MaintainsStackDepthParentAndAncestors)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Walker Ancestry for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstWalkerAncestryParameterizedTest,
        testing::ValuesIn((AstWalkerDescriptorCollector<AllAstNodeTypes, AstWalkerAncestryTestBuilder>::collect())),
        AstWalkerTestNameGenerator{}
    );
}
