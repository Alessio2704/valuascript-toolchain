#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/ast.h"

using namespace valuascript;
using namespace valuascript::compiler;

class AstMathPrecedenceTest : public testing::Test {
protected:
    std::shared_ptr<Program> parse_code(const std::string &code) {
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

        return std::any_cast<std::shared_ptr<Program> >(parser_result.data);
    }

    Expression *get_root_expr(const std::shared_ptr<Program> &ast) {
        auto const assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        return assignment->value.get();
    }
};


TEST_F(AstMathPrecedenceTest, ValidatesStandardPrecedence) {
    /*
     Expected AST Shape:
             (+)
            /   \
          (1)   (*)
               /   \
             (2)   (^)
                  /   \
                (3)   (4)
    */

    auto ast = parse_code("let a = 1 + 2 * 3 ^ 4");
    auto root_expr = get_root_expr(ast);

    // 1. Root must be Addition
    auto add_node = dynamic_cast<BinaryExpression *>(root_expr);
    ASSERT_NE(add_node, nullptr) << "Root is not a BinaryExpression";
    EXPECT_EQ(add_node->op, TokenType::Plus);

    // 2. Left of Addition is 1
    auto num_1 = dynamic_cast<NumberLiteral *>(add_node->left.get());
    ASSERT_NE(num_1, nullptr);
    EXPECT_EQ(num_1->value, "1");

    // 3. Right of Addition must be Multiplication
    auto mul_node = dynamic_cast<BinaryExpression *>(add_node->right.get());
    ASSERT_NE(mul_node, nullptr);
    EXPECT_EQ(mul_node->op, TokenType::Star);

    // 4. Right of Multiplication must be Power
    auto pow_node = dynamic_cast<BinaryExpression *>(mul_node->right.get());
    ASSERT_NE(pow_node, nullptr);
    EXPECT_EQ(pow_node->op, TokenType::Caret);
}

TEST_F(AstMathPrecedenceTest, ValidatesLeftAssociativitySubtraction) {
    /*
     Code: let a = 10 - 5 - 2
     Expected AST Shape (Left-Associative):
             (-)
            /   \
          (-)   (2)
         /   \
      (10)   (5)
    */

    auto ast = parse_code("let a = 10 - 5 - 2");
    auto outer_sub = dynamic_cast<BinaryExpression *>(get_root_expr(ast));

    ASSERT_NE(outer_sub, nullptr);
    EXPECT_EQ(outer_sub->op, TokenType::Minus);

    auto right_leaf = dynamic_cast<NumberLiteral *>(outer_sub->right.get());
    ASSERT_NE(right_leaf, nullptr);
    EXPECT_EQ(right_leaf->value, "2");

    auto inner_sub = dynamic_cast<BinaryExpression *>(outer_sub->left.get());
    ASSERT_NE(inner_sub, nullptr);
    EXPECT_EQ(inner_sub->op, TokenType::Minus);

    auto inner_left = dynamic_cast<NumberLiteral *>(inner_sub->left.get());
    ASSERT_NE(inner_left, nullptr);
    EXPECT_EQ(inner_left->value, "10");
}

TEST_F(AstMathPrecedenceTest, ValidatesLeftAssociativityDivision) {
    /*
     Evaluates as (20 / 5) / 2 = 2, NOT 20 / (5 / 2).
     Expected AST:
             (/)
            /   \
          (/)   (2)
         /   \
      (20)   (5)
    */

    auto ast = parse_code("let a = 20 / 5 / 2");
    auto outer_div = dynamic_cast<BinaryExpression *>(get_root_expr(ast));

    ASSERT_NE(outer_div, nullptr);
    EXPECT_EQ(outer_div->op, TokenType::Slash);

    auto right_leaf = dynamic_cast<NumberLiteral *>(outer_div->right.get());
    ASSERT_NE(right_leaf, nullptr);
    EXPECT_EQ(right_leaf->value, "2");

    auto inner_div = dynamic_cast<BinaryExpression *>(outer_div->left.get());
    ASSERT_NE(inner_div, nullptr);
    EXPECT_EQ(inner_div->op, TokenType::Slash);
}

TEST_F(AstMathPrecedenceTest, ValidatesUnaryMinusPrecedence) {
    /*
      Unary minus has higher precedence than multiplication.
      Expected AST:
             (*)
            /   \
          (-)   (2)
           |
          (5)
     */

    auto ast = parse_code("let a = -5 * 2");
    auto mul_node = dynamic_cast<BinaryExpression *>(get_root_expr(ast));

    ASSERT_NE(mul_node, nullptr);
    EXPECT_EQ(mul_node->op, TokenType::Star);

    auto unary_node = dynamic_cast<UnaryExpression *>(mul_node->left.get());
    ASSERT_NE(unary_node, nullptr);
    EXPECT_EQ(unary_node->op, TokenType::Minus);

    auto num_5 = dynamic_cast<NumberLiteral *>(unary_node->right.get());
    ASSERT_NE(num_5, nullptr);
    EXPECT_EQ(num_5->value, "5");
}

TEST_F(AstMathPrecedenceTest, ValidatesParenthesesOverride) {
    /*
     Expected AST Shape:
           (*)
          /   \
        (+)   (3)
       /   \
     (1)   (2)
    */

    auto ast = parse_code("let a = (1 + 2) * 3");
    auto mul_node = dynamic_cast<BinaryExpression *>(get_root_expr(ast));

    ASSERT_NE(mul_node, nullptr) << "Root is not Multiplication";
    EXPECT_EQ(mul_node->op, TokenType::Star);

    auto add_node = dynamic_cast<BinaryExpression *>(mul_node->left.get());
    ASSERT_NE(add_node, nullptr) << "Left branch is not Addition";
    EXPECT_EQ(add_node->op, TokenType::Plus);
}

TEST_F(AstMathPrecedenceTest, ValidatesDeepParenthesesNesting) {
    /*
     Expected AST Shape:
                (/)
               /   \
             (*)   (5)
            /   \
          (+)   (-)
         /  \   /  \
       (1) (2)(3) (4)
    */

    auto ast = parse_code("let a = ((1 + 2) * (3 - 4)) / 5");
    auto div_node = dynamic_cast<BinaryExpression *>(get_root_expr(ast));

    ASSERT_NE(div_node, nullptr);
    EXPECT_EQ(div_node->op, TokenType::Slash);

    // Left of division is the multiplication
    auto mul_node = dynamic_cast<BinaryExpression *>(div_node->left.get());
    ASSERT_NE(mul_node, nullptr);
    EXPECT_EQ(mul_node->op, TokenType::Star);

    // Left of multiplication is addition
    auto add_node = dynamic_cast<BinaryExpression *>(mul_node->left.get());
    ASSERT_NE(add_node, nullptr);
    EXPECT_EQ(add_node->op, TokenType::Plus);

    // Right of multiplication is subtraction
    auto sub_node = dynamic_cast<BinaryExpression *>(mul_node->right.get());
    ASSERT_NE(sub_node, nullptr);
    EXPECT_EQ(sub_node->op, TokenType::Minus);
}

TEST_F(AstMathPrecedenceTest, ValidatesRelationalPrecedence) {
    /*
     Expected AST Shape:
              (>)
             /   \
           (+)   (*)
          /  \   /  \
        (1) (2)(3)  (4)
     Relational operators must have lower precedence than + and *
    */

    auto ast = parse_code("let a = 1 + 2 > 3 * 4");
    auto root_expr = dynamic_cast<BinaryExpression *>(get_root_expr(ast));

    // 1. Root MUST be Greater Than
    ASSERT_NE(root_expr, nullptr);
    EXPECT_EQ(root_expr->op, TokenType::Greater);

    // 2. Left MUST be Addition
    auto left_add = dynamic_cast<BinaryExpression *>(root_expr->left.get());
    ASSERT_NE(left_add, nullptr);
    EXPECT_EQ(left_add->op, TokenType::Plus);

    // 3. Right MUST be Multiplication
    auto right_mul = dynamic_cast<BinaryExpression *>(root_expr->right.get());
    ASSERT_NE(right_mul, nullptr);
    EXPECT_EQ(right_mul->op, TokenType::Star);
}

TEST_F(AstMathPrecedenceTest, ValidatesEqualityPrecedence) {
    /*
     Expected AST Shape:
              (==)
             /    \
           (x)    (+)
                 /   \
               (y)   (1)
    */

    auto ast = parse_code("let a = x == y + 1");
    auto root_expr = dynamic_cast<BinaryExpression *>(get_root_expr(ast));

    // 1. Root MUST be Equals
    ASSERT_NE(root_expr, nullptr);
    EXPECT_EQ(root_expr->op, TokenType::Equals);

    // 2. Left MUST be identifier 'x'
    auto left_id = dynamic_cast<IdentifierAccess *>(root_expr->left.get());
    ASSERT_NE(left_id, nullptr);
    EXPECT_EQ(left_id->name, "x");

    // 3. Right MUST be Addition
    auto right_add = dynamic_cast<BinaryExpression *>(root_expr->right.get());
    ASSERT_NE(right_add, nullptr);
    EXPECT_EQ(right_add->op, TokenType::Plus);
}

TEST_F(AstMathPrecedenceTest, ValidatesModuloPrecedence) {
    /*
     Expected AST Shape:
              (*)
             /   \
           (mod)   (2)
          /   \
       (10)   (3)
     Evaluates as (10 mod 3) * 2 = 2. If it were right-associative, it would be 10 mod (3 * 2) = 4.
    */

    auto ast = parse_code("let a = 10 mod 3 * 2");
    auto root_expr = dynamic_cast<BinaryExpression *>(get_root_expr(ast));

    // 1. Root MUST be Multiplication (due to left-associativity of same-precedence operators)
    ASSERT_NE(root_expr, nullptr);
    EXPECT_EQ(root_expr->op, TokenType::Star);

    // 2. Left MUST be Modulo
    auto left_mod = dynamic_cast<BinaryExpression *>(root_expr->left.get());
    ASSERT_NE(left_mod, nullptr);
    EXPECT_EQ(left_mod->op, TokenType::Mod);

    auto mod_left = dynamic_cast<NumberLiteral *>(left_mod->left.get());
    auto mod_right = dynamic_cast<NumberLiteral *>(left_mod->right.get());
    ASSERT_NE(mod_left, nullptr);
    ASSERT_NE(mod_right, nullptr);
    EXPECT_EQ(mod_left->value, "10");
    EXPECT_EQ(mod_right->value, "3");
}
