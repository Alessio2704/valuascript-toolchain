#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/ast.h"

using namespace valuascript;
using namespace valuascript::compiler;

class AstUnaryExpressionTest : public testing::Test {
protected:
    std::shared_ptr<Program> parse_code(const std::string& code) {
        LexerStage lexer;
        auto lexer_result = lexer.run({
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, code}
        });

        ParserStage parser;
        auto parser_result = parser.run({
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            lexer_result
        });
        
        return std::any_cast<std::shared_ptr<Program>>(parser_result.data);
    }
    
    Expression* get_assigned_value(const std::shared_ptr<Program>& ast) {
        if (ast->execution_steps.empty()) return nullptr;
        auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
        if (!assign) return nullptr;
        return assign->value.get();
    }
};

TEST_F(AstUnaryExpressionTest, ValidatesArithmeticNegation) {
    // Proves the parser correctly identifies a unary minus instead of a binary subtraction.
    
    auto ast = parse_code("let neg = -5");
    auto unary_op = dynamic_cast<UnaryExpression*>(get_assigned_value(ast));
    
    ASSERT_NE(unary_op, nullptr) << "Assigned value must be a UnaryExpression";
    EXPECT_EQ(unary_op->op, TokenType::Minus);
    
    auto right_val = dynamic_cast<NumberLiteral*>(unary_op->right.get());
    ASSERT_NE(right_val, nullptr) << "Operand must be a NumberLiteral";
    EXPECT_EQ(right_val->value, "5");
}

TEST_F(AstUnaryExpressionTest, ValidatesLogicalInversion) {
    // Proves the logical NOT operator binds correctly to an identifier.
    
    auto ast = parse_code("let inv = not is_active");
    auto unary_op = dynamic_cast<UnaryExpression*>(get_assigned_value(ast));
    
    ASSERT_NE(unary_op, nullptr);
    EXPECT_EQ(unary_op->op, TokenType::Not);
    
    auto right_val = dynamic_cast<IdentifierAccess*>(unary_op->right.get());
    ASSERT_NE(right_val, nullptr);
    EXPECT_EQ(right_val->name, "is_active");
}

TEST_F(AstUnaryExpressionTest, ValidatesUnaryPrecedenceOverBinary) {
    // Proves that unary minus binds tighter than multiplication.
    // The AST root MUST be '*', and its left child MUST be the UnaryExpression '-a'.
    
    auto ast = parse_code("let math = -a * b");
    auto root_math = dynamic_cast<BinaryExpression*>(get_assigned_value(ast));
    
    ASSERT_NE(root_math, nullptr) << "Root must be the BinaryExpression (*), not the UnaryExpression";
    EXPECT_EQ(root_math->op, TokenType::Star);
    
    // Verify Left Child: UnaryExpression (-a)
    auto left_unary = dynamic_cast<UnaryExpression*>(root_math->left.get());
    ASSERT_NE(left_unary, nullptr) << "Left side of multiplication must be the UnaryExpression";
    EXPECT_EQ(left_unary->op, TokenType::Minus);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(left_unary->right.get())->name, "a");
    
    // Verify Right Child: Identifier (b)
    auto right_id = dynamic_cast<IdentifierAccess*>(root_math->right.get());
    ASSERT_NE(right_id, nullptr);
    EXPECT_EQ(right_id->name, "b");
}

TEST_F(AstUnaryExpressionTest, ValidatesNestedUnaryChaining) {
    // Proves that unary operators can recursively stack on themselves perfectly right-to-left.
    
    auto ast = parse_code("let double_neg = not not flag");
    auto outer_unary = dynamic_cast<UnaryExpression*>(get_assigned_value(ast));
    
    ASSERT_NE(outer_unary, nullptr);
    EXPECT_EQ(outer_unary->op, TokenType::Not);
    
    // The operand of the first '!' must be ANOTHER UnaryExpression
    auto inner_unary = dynamic_cast<UnaryExpression*>(outer_unary->right.get());
    ASSERT_NE(inner_unary, nullptr) << "Operand of outer unary must be the inner UnaryExpression";
    EXPECT_EQ(inner_unary->op, TokenType::Not);
    
    // The operand of the second '!' is the target identifier
    auto target_id = dynamic_cast<IdentifierAccess*>(inner_unary->right.get());
    ASSERT_NE(target_id, nullptr);
    EXPECT_EQ(target_id->name, "flag");
}

TEST_F(AstUnaryExpressionTest, ValidatesDeeplyNestedUnaryAndBinaryMath) {
    // Proves flawless execution of the Pratt parsing precedence table across
    // unary binding, mathematical groupings, high-precedence binary operators (*, /),
    // and low-precedence binary operators (-).

    auto ast = parse_code("let complex = -a * (b + c) - (not d) / e");

    // 1. Root MUST be the lowest precedence binary operator: Subtraction (-)
    auto root_sub = dynamic_cast<BinaryExpression*>(get_assigned_value(ast));
    ASSERT_NE(root_sub, nullptr) << "Root must be a BinaryExpression (Subtraction)";
    EXPECT_EQ(root_sub->op, TokenType::Minus);

    // ==========================================
    // LEFT BRANCH: -a * (b + c)
    // ==========================================
    auto left_mult = dynamic_cast<BinaryExpression*>(root_sub->left.get());
    ASSERT_NE(left_mult, nullptr) << "Left branch of subtraction must be Multiplication";
    EXPECT_EQ(left_mult->op, TokenType::Star);

    // Left of *: Unary Minus (-a)
    auto unary_minus = dynamic_cast<UnaryExpression*>(left_mult->left.get());
    ASSERT_NE(unary_minus, nullptr) << "Left side of multiplication must be a UnaryExpression";
    EXPECT_EQ(unary_minus->op, TokenType::Minus);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(unary_minus->right.get())->name, "a");

    // Right of *: Grouped Addition (b + c)
    // Parentheses must be stripped, leaving just the BinaryExpression
    auto grouped_add = dynamic_cast<BinaryExpression*>(left_mult->right.get());
    ASSERT_NE(grouped_add, nullptr) << "Right side of multiplication must be the Grouped Addition";
    EXPECT_EQ(grouped_add->op, TokenType::Plus);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(grouped_add->left.get())->name, "b");
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(grouped_add->right.get())->name, "c");

    // ==========================================
    // RIGHT BRANCH: !d / e
    // ==========================================
    auto right_div = dynamic_cast<BinaryExpression*>(root_sub->right.get());
    ASSERT_NE(right_div, nullptr) << "Right branch of subtraction must be Division";
    EXPECT_EQ(right_div->op, TokenType::Slash);

    // Left of /: Unary NOT (!d)
    auto unary_not = dynamic_cast<UnaryExpression*>(right_div->left.get());
    ASSERT_NE(unary_not, nullptr) << "Left side of division must be a UnaryExpression";
    EXPECT_EQ(unary_not->op, TokenType::Not);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(unary_not->right.get())->name, "d");

    // Right of /: Identifier (e)
    auto id_e = dynamic_cast<IdentifierAccess*>(right_div->right.get());
    ASSERT_NE(id_e, nullptr) << "Right side of division must be an Identifier";
    EXPECT_EQ(id_e->name, "e");
}

TEST_F(AstUnaryExpressionTest, ValidatesDeeplyNestedUnaryAndBinaryMath_2) {
    // This is a regression test variant of the above -> it has no parenthesis around the not

    auto ast = parse_code("let complex = -a * (b + c) - not d / e");

    // 1. Root MUST be the lowest precedence binary operator: Subtraction (-)
    auto root_sub = dynamic_cast<BinaryExpression*>(get_assigned_value(ast));
    ASSERT_NE(root_sub, nullptr) << "Root must be a BinaryExpression (Subtraction)";
    EXPECT_EQ(root_sub->op, TokenType::Minus);

    // ==========================================
    // LEFT BRANCH: -a * (b + c)
    // ==========================================
    auto left_mult = dynamic_cast<BinaryExpression*>(root_sub->left.get());
    ASSERT_NE(left_mult, nullptr) << "Left branch of subtraction must be Multiplication";
    EXPECT_EQ(left_mult->op, TokenType::Star);

    // Left of *: Unary Minus (-a)
    auto unary_minus = dynamic_cast<UnaryExpression*>(left_mult->left.get());
    ASSERT_NE(unary_minus, nullptr) << "Left side of multiplication must be a UnaryExpression";
    EXPECT_EQ(unary_minus->op, TokenType::Minus);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(unary_minus->right.get())->name, "a");

    // Right of *: Grouped Addition (b + c)
    // Parentheses must be stripped, leaving just the BinaryExpression
    auto grouped_add = dynamic_cast<BinaryExpression*>(left_mult->right.get());
    ASSERT_NE(grouped_add, nullptr) << "Right side of multiplication must be the Grouped Addition";
    EXPECT_EQ(grouped_add->op, TokenType::Plus);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(grouped_add->left.get())->name, "b");
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(grouped_add->right.get())->name, "c");

    // ==========================================
    // RIGHT BRANCH: !d / e
    // ==========================================
    auto right_div = dynamic_cast<BinaryExpression*>(root_sub->right.get());
    ASSERT_NE(right_div, nullptr) << "Right branch of subtraction must be Division";
    EXPECT_EQ(right_div->op, TokenType::Slash);

    // Left of /: Unary NOT (!d)
    auto unary_not = dynamic_cast<UnaryExpression*>(right_div->left.get());
    ASSERT_NE(unary_not, nullptr) << "Left side of division must be a UnaryExpression";
    EXPECT_EQ(unary_not->op, TokenType::Not);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(unary_not->right.get())->name, "d");

    // Right of /: Identifier (e)
    auto id_e = dynamic_cast<IdentifierAccess*>(right_div->right.get());
    ASSERT_NE(id_e, nullptr) << "Right side of division must be an Identifier";
    EXPECT_EQ(id_e->name, "e");
}