#include "stages/frontend/parser/ast_base_test.h"

namespace valuascript::compiler::test {
    TEST_F(AstBaseTest, ValidatesDirectiveWithNoValue) {
        // Proves the parser correctly identifies the directive name and leaves the value pointer null.

        auto ast = parse_code("#strict");
        auto directive = get_directive(ast);

        ASSERT_NE(directive, nullptr) << "Execution step must be a Directive node";
        EXPECT_EQ(directive->name, "strict");

        // Value must be implicitly null
        EXPECT_EQ(directive->value.get(), nullptr) << "Directive without a value must have a null value pointer";
    }

    TEST_F(AstBaseTest, ValidatesDirectiveWithValueWithoutEquals) {
        // Proves the parser captures a trailing expression even if '=' is omitted.

        auto ast = parse_code("#no_equal \"macro_assumptions.vs\"");
        auto directive = get_directive(ast);

        ASSERT_NE(directive, nullptr);
        EXPECT_EQ(directive->name, "no_equal");

        // Verify the value is correctly parsed as a StringLiteral
        auto val = dynamic_cast<StringLiteral *>(directive->value.get());
        ASSERT_NE(val, nullptr) << "Directive value must be a parsed StringLiteral";

        // Assuming you implemented the quote-stripping fix earlier
        EXPECT_EQ(val->value, "\"macro_assumptions.vs\"");
    }

    TEST_F(AstBaseTest, ValidatesDirectiveWithValueWithEquals) {
        // Proves the parser gracefully consumes the optional '=' and accurately captures the right-hand value.

        auto ast = parse_code("#default_wacc = 0.08");
        auto directive = get_directive(ast);

        ASSERT_NE(directive, nullptr);
        EXPECT_EQ(directive->name, "default_wacc");

        // Verify the value is correctly parsed as a NumberLiteral
        auto val = dynamic_cast<NumberLiteral *>(directive->value.get());
        ASSERT_NE(val, nullptr) << "Directive value must be a parsed NumberLiteral";
        EXPECT_EQ(val->value, "0.08");
    }

    TEST_F(AstBaseTest, ValidatesDirectiveWithComplexExpressionValue) {
        // Proves the directive value isn't limited to primitives, but accepts full expressions.

        auto ast = parse_code("#timeout = 60 * 5");
        auto directive = get_directive(ast);

        ASSERT_NE(directive, nullptr);
        EXPECT_EQ(directive->name, "timeout");

        // Verify the value is correctly parsed as a BinaryExpression
        auto val = dynamic_cast<BinaryExpression *>(directive->value.get());
        ASSERT_NE(val, nullptr) << "Directive value must be a BinaryExpression";
        EXPECT_EQ(val->op, TokenType::Star);

        EXPECT_EQ(dynamic_cast<NumberLiteral*>(val->left.get())->value, "60");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(val->right.get())->value, "5");
    }
}
