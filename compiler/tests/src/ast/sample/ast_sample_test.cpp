#include <gtest/gtest.h>
#include <string>

#include "ast_sample_test_helper.h"

namespace valuascript::compiler::test
{
    class AstSampleFactoryParameterizedTest : public testing::TestWithParam<AstSampleTestDescriptor>
    {
    };

    TEST_P(AstSampleFactoryParameterizedTest, NodeInstantiationAndValidityInvariants)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing AST Sample Factory for: " + descriptor.node_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstSampleTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstSampleTestDescriptor>& info) const
        {
            return info.param.node_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstSampleFactoryParameterizedTest,
        testing::ValuesIn(get_all_ast_sample_test_descriptors()),
        AstSampleTestNameGenerator{}
    );

    TEST(AstSampleFactorySuite, DeterminismAcrossResets)
    {
        reset_sample_generator_state(250);
        auto p1 = create_sample_program();

        reset_sample_generator_state(250);
        auto p2 = create_sample_program();

        ASSERT_NE(p1, nullptr);
        ASSERT_NE(p2, nullptr);
        EXPECT_NE(p1.get(), p2.get());
        EXPECT_EQ(p1->span, p2->span);
        EXPECT_EQ(p1->kind, AstKind::Program);
        EXPECT_EQ(p2->kind, AstKind::Program);
        EXPECT_TRUE(p1->is_valid());
        EXPECT_TRUE(p2->is_valid());
    }

    TEST(AstSampleFactorySuite, DepthBounding)
    {
        for (int depth = 0; depth <= 3; ++depth)
        {
            auto expr = sample_expr(depth);
            ASSERT_NE(expr, nullptr);
            EXPECT_TRUE(expr->is_valid());

            auto stmt = sample_stmt(depth);
            ASSERT_NE(stmt, nullptr);
            EXPECT_TRUE(stmt->is_valid());

            auto type = sample_type(depth);
            ASSERT_NE(type, nullptr);
            EXPECT_TRUE(type->is_valid());
        }
    }
}
