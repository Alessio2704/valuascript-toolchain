#include "../ast_base_test.h"
using namespace valuascript::compiler::test;

TEST_F(AstBaseTest, MissingLeadingZeroOnDecimalPartialAST) {
    auto ast = parse_code("let a = .5", false);

    auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
    ASSERT_NE(assignment, nullptr);

    auto num_literal = dynamic_cast<NumberLiteral *>(assignment->value.get());
    ASSERT_NE(num_literal, nullptr);
    ASSERT_EQ(num_literal->value, ".5");
}

TEST_F(AstBaseTest, MissingLeadingZeroOnDecimalPercentageLiteralPartialAST) {
    auto ast = parse_code("let a = .5%", false);

    auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
    ASSERT_NE(assignment, nullptr);

    auto percentage_literal = dynamic_cast<PercentageLiteral *>(assignment->value.get());
    ASSERT_NE(percentage_literal, nullptr);
    ASSERT_EQ(percentage_literal->value, ".5%");
}

