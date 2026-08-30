#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <utility>

#include "ast_rewriter_test_helper.h"

namespace valuascript::compiler::test
{
    template <typename NodeT>
    inline void run_identity_rewrite_test()
    {
        AstRewriter rewriter;
        auto sample = create_sample<NodeT>(2);
        auto cloned = clone_node(sample);

        if constexpr (valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>)
        {
            auto rewritten = rewriter.rewrite(std::move(cloned));
            EXPECT_TRUE(ast_equals(rewritten, sample));
        }
        else
        {
            ASSERT_NE(sample, nullptr);
            ASSERT_NE(cloned, nullptr);

            auto rewritten = rewriter.rewrite(std::move(cloned));
            ASSERT_NE(rewritten, nullptr);
            EXPECT_TRUE(ast_equals(rewritten, sample));
        }
    }

    template <typename NodeT>
    struct AstRewriterIdentityTestBuilder
    {
        static AstRewriterTestDescriptor build()
        {
            return AstRewriterTestDescriptor{
                .name = std::string(get_ast_node_name<NodeT>()),
                .run_test = []()
                {
                    run_identity_rewrite_test<NodeT>();
                }
            };
        }
    };

    class AstRewriterIdentityParameterizedTest : public testing::TestWithParam<AstRewriterTestDescriptor>
    {
    };

    TEST_P(AstRewriterIdentityParameterizedTest, DefaultRewriterPreservesNodeIdentity)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Identity Rewriting for: " + descriptor.name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstRewriterIdentityParameterizedTest,
        testing::ValuesIn((AstRewriterDescriptorCollector<AllAstNodeTypes, AstRewriterIdentityTestBuilder>::collect())),
        AstRewriterTestNameGenerator{}
    );
}
