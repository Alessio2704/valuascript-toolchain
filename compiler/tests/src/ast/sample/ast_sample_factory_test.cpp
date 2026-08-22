#include <gtest/gtest.h>
#include <string>

#include "ast_sample_test_factory.h"

namespace valuascript::compiler::test
{
    class AstSampleFactoryParameterizedTest : public testing::TestWithParam<AstSampleTestDescriptor>
    {
    };

    TEST_P(AstSampleFactoryParameterizedTest, NodeInstantiationAndInvariants)
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

        EXPECT_TRUE(ast_equals(p1, p2));
        EXPECT_TRUE(ast_is_disjoint(p1, p2));
    }

    TEST(AstSampleFactorySuite, DepthBounding)
    {
        for (int depth = 0; depth <= 3; ++depth)
        {
            auto expr = sample_expr(depth);
            ASSERT_NE(expr, nullptr);
            EXPECT_GT(expr->span.line_start, 0);

            auto stmt = sample_stmt(depth);
            ASSERT_NE(stmt, nullptr);
            EXPECT_GT(stmt->span.line_start, 0);

            auto type = sample_type(depth);
            ASSERT_NE(type, nullptr);
            EXPECT_GT(type->span.line_start, 0);
        }
    }

    TEST(AstSampleFactorySuite, SampleProgramCompleteness)
    {
        auto prog = create_sample_program();
        ASSERT_NE(prog, nullptr);

        EXPECT_FALSE(prog->comments.empty());
        EXPECT_FALSE(prog->import_statements.empty());
        EXPECT_FALSE(prog->directives.empty());
        EXPECT_FALSE(prog->execution_steps.empty());
        EXPECT_FALSE(prog->function_definitions.empty());
        EXPECT_FALSE(prog->struct_definitions.empty());
        EXPECT_FALSE(prog->enum_definitions.empty());
        EXPECT_FALSE(prog->type_aliases.empty());
        EXPECT_FALSE(prog->extension_definitions.empty());
    }
}
