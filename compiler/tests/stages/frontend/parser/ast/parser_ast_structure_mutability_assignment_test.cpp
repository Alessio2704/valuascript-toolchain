#include "../ast_base_test.h"
using namespace valuascript::compiler::test;

TEST_F(AstBaseTest, LetKeywordProducesImmutableNode) {
    auto ast = parse_code("let tax_rate = 20%");

    ASSERT_EQ(ast->execution_steps.size(), 1);
    auto declaration = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(declaration, nullptr);

    EXPECT_EQ(declaration->targets[0].first, "tax_rate");
    EXPECT_FALSE(declaration->is_mutable) << "Expected 'let' to flag the node as immutable.";
}

TEST_F(AstBaseTest, VarKeywordProducesMutableNode) {
    // Exact same structure, different keyword
    auto ast = parse_code("var index = 0");

    ASSERT_EQ(ast->execution_steps.size(), 1);
    auto declaration = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(declaration, nullptr);

    EXPECT_EQ(declaration->targets[0].first, "index");
    EXPECT_TRUE(declaration->is_mutable) << "Expected 'var' to flag the node as mutable.";
}

TEST_F(AstBaseTest, LetKeywordDistributesImmutabilityToAllTargets) {
    auto ast = parse_code("let base_rate, risk_premium = my_func()");

    ASSERT_EQ(ast->execution_steps.size(), 1);

    auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
    ASSERT_NE(assignment, nullptr);

    // Verify the targets were captured
    ASSERT_EQ(assignment->targets.size(), 2);
    EXPECT_EQ(assignment->targets[0].first, "base_rate");
    EXPECT_EQ(assignment->targets[1].first, "risk_premium");

    // The critical assertion: The entire node must be flagged as immutable
    EXPECT_FALSE(assignment->is_mutable) << "Expected 'let' to flag all targets as immutable.";
}

TEST_F(AstBaseTest, VarKeywordDistributesMutabilityToAllTargets) {
    auto ast = parse_code("var base_rate, risk_premium = my_func()");

    ASSERT_EQ(ast->execution_steps.size(), 1);

    auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
    ASSERT_NE(assignment, nullptr);

    // Verify the targets were captured
    ASSERT_EQ(assignment->targets.size(), 2);
    EXPECT_EQ(assignment->targets[0].first, "base_rate");
    EXPECT_EQ(assignment->targets[1].first, "risk_premium");

    // The critical assertion: The entire node must be flagged as immutable
    EXPECT_TRUE(assignment->is_mutable) << "Expected 'var' to flag all targets as mutable.";
}