#include <gtest/gtest.h>
#include "ast/equality/ast_disjoint.h"
#include "ast/sample/ast_sample_factory.h"

namespace valuascript::compiler::test
{
    TEST(AstDisjointTest, IndependentlyConstructedTreesAreDisjoint)
    {
        reset_sample_generator_state();
        auto prog1 = create_sample_program();

        reset_sample_generator_state();
        auto prog2 = create_sample_program();

        EXPECT_TRUE(ast_is_disjoint(prog1, prog2));
        EXPECT_TRUE(ast_is_disjoint(prog2, prog1));
    }

    TEST(AstDisjointTest, SelfComparisonReturnsFalseForNodesWithPointers)
    {
        reset_sample_generator_state();
        auto prog = create_sample_program();
        EXPECT_FALSE(ast_is_disjoint(prog, prog));
    }

    TEST(AstDisjointTest, AliasedPointerDetected)
    {
        auto left_ptr = std::make_unique<NumberLiteral>("1");
        auto right_ptr = std::make_unique<NumberLiteral>("2");

        auto bin1 = std::make_unique<BinaryExpression>(
            std::move(left_ptr), TokenType::Plus, std::move(right_ptr));

        auto bin2 = std::make_unique<BinaryExpression>(
            std::make_unique<NumberLiteral>("1"), TokenType::Plus, std::make_unique<NumberLiteral>("2"));

        EXPECT_TRUE(ast_is_disjoint(bin1, bin2));

        auto aliased = std::make_unique<BinaryExpression>(
            std::make_unique<NumberLiteral>("1"), TokenType::Plus, nullptr);
        aliased->right = std::unique_ptr<Expression>(bin1->right.get());

        EXPECT_FALSE(ast_is_disjoint(bin1, aliased));

        (void)aliased->right.release();
    }

    TEST(AstDisjointTest, NullptrComparisonsAreDisjoint)
    {
        std::unique_ptr<NumberLiteral> p1 = nullptr;
        std::unique_ptr<NumberLiteral> p2 = nullptr;
        auto p3 = std::make_unique<NumberLiteral>("10");

        EXPECT_TRUE(ast_is_disjoint(p1, p2));
        EXPECT_TRUE(ast_is_disjoint(p1, p3));
        EXPECT_TRUE(ast_is_disjoint(p3, p1));
    }

    TEST(AstDisjointTest, PolymorphicBasePointerDisjointness)
    {
        reset_sample_generator_state();
        auto func1 = create_sample<FunctionDefinition>();

        reset_sample_generator_state();
        std::unique_ptr<AstNode> base_func2 = create_sample<FunctionDefinition>();

        EXPECT_TRUE(ast_is_disjoint(func1.get(), base_func2.get()));
        EXPECT_FALSE(ast_is_disjoint(func1.get(), func1.get()));
    }
}
