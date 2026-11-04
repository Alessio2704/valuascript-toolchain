#include <gtest/gtest.h>
#include "ast/equality/ast_equality.h"
#include "ast/factory/ast_factory.h"

namespace valuascript::compiler::test
{
    TEST(AstEqualityBaselineTest, ReflexivityAcrossSampleProgram)
    {
        reset_factory_state();
        auto prog = create_sample_program();
        EXPECT_TRUE(ast_equals(prog, prog));
    }

    TEST(AstEqualityBaselineTest, IdenticallyConstructedProgramsAreEqual)
    {
        reset_factory_state();
        auto prog1 = create_sample_program();

        reset_factory_state();
        auto prog2 = create_sample_program();

        EXPECT_TRUE(ast_equals(prog1, prog2));
        EXPECT_TRUE(ast_equals(prog2, prog1));
    }

    TEST(AstEqualityBaselineTest, NullptrEqualityComparisons)
    {
        std::unique_ptr<NumberLiteral> p1 = nullptr;
        std::unique_ptr<NumberLiteral> p2 = nullptr;
        auto p3 = std::make_unique<NumberLiteral>("10");

        EXPECT_TRUE(ast_equals(p1, p2));
        EXPECT_FALSE(ast_equals(p1, p3));
        EXPECT_FALSE(ast_equals(p3, p1));
    }

    TEST(AstEqualityBaselineTest, PolymorphicBasePointerEquality)
    {
        reset_factory_state();
        auto func1 = create_sample<FunctionDefinition>(0);

        reset_factory_state();
        std::unique_ptr<AstNode> base_func2 = create_sample<FunctionDefinition>(0);

        EXPECT_TRUE(ast_equals(func1.get(), base_func2.get()));
        EXPECT_TRUE(ast_equals(base_func2.get(), func1.get()));
    }
}
