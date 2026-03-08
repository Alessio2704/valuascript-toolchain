#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/ast.h"

using namespace valuascript;
using namespace valuascript::compiler;


class AstBooleanAndConditionalsTest : public testing::Test {
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

    Expression* get_root_expr(const std::shared_ptr<Program>& ast) {
        auto const assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        return assignment->value.get();
    }
};

TEST_F(AstBooleanAndConditionalsTest, ValidatesBooleanPrecedence) {
    /*
     Expected AST Shape:
             (or)
            /    \
          (x)   (and)
               /     \
             (y)    (not)
                      |
                     (z)
    */

    auto ast = parse_code("let a = x or y and not z");
    auto root_expr = get_root_expr(ast);

    // 1. Root is OR
    auto or_node = dynamic_cast<BinaryExpression*>(root_expr);
    ASSERT_NE(or_node, nullptr);
    EXPECT_EQ(or_node->op, TokenType::Or);

    // 2. Left is 'x'
    auto id_x = dynamic_cast<IdentifierAccess*>(or_node->left.get());
    ASSERT_NE(id_x, nullptr);
    EXPECT_EQ(id_x->name, "x");

    // 3. Right is AND
    auto and_node = dynamic_cast<BinaryExpression*>(or_node->right.get());
    ASSERT_NE(and_node, nullptr);
    EXPECT_EQ(and_node->op, TokenType::And);

    // 4. AND's Left is 'y'
    auto id_y = dynamic_cast<IdentifierAccess*>(and_node->left.get());
    ASSERT_NE(id_y, nullptr);
    EXPECT_EQ(id_y->name, "y");

    // 5. AND's Right is NOT
    auto not_node = dynamic_cast<UnaryExpression*>(and_node->right.get());
    ASSERT_NE(not_node, nullptr);
    EXPECT_EQ(not_node->op, TokenType::Not);

    // 6. NOT's operand is 'z'
    auto id_z = dynamic_cast<IdentifierAccess*>(not_node->right.get());
    ASSERT_NE(id_z, nullptr);
    EXPECT_EQ(id_z->name, "z");
}

TEST_F(AstBooleanAndConditionalsTest, ValidatesBooleanGrouping) {
    /*
     Expected AST Shape:
             (and)
            /     \
          (or)    (z)
         /    \
       (x)    (y)
    */

    auto ast = parse_code("let a = (x or y) and z");
    auto root_expr = get_root_expr(ast);

    auto and_node = dynamic_cast<BinaryExpression*>(root_expr);
    ASSERT_NE(and_node, nullptr);
    EXPECT_EQ(and_node->op, TokenType::And);

    auto or_node = dynamic_cast<BinaryExpression*>(and_node->left.get());
    ASSERT_NE(or_node, nullptr);
    EXPECT_EQ(or_node->op, TokenType::Or);
}

TEST_F(AstBooleanAndConditionalsTest, ValidatesSimpleConditional) {
    /*
     Expected AST Shape:
     Conditional
      |- Cond: (>)
      |       /   \
      |     (x)   (0)
      |- Then: (10)
      |- Else: (20)
    */

    auto ast = parse_code("let a = if x > 0 then 10 else 20");
    auto root_expr = get_root_expr(ast);

    auto cond_expr = dynamic_cast<ConditionalExpression*>(root_expr);
    ASSERT_NE(cond_expr, nullptr);

    auto cmp_node = dynamic_cast<BinaryExpression*>(cond_expr->condition.get());
    ASSERT_NE(cmp_node, nullptr);
    EXPECT_EQ(cmp_node->op, TokenType::Greater);

    auto then_node = dynamic_cast<NumberLiteral*>(cond_expr->then_branch.get());
    ASSERT_NE(then_node, nullptr);
    EXPECT_EQ(then_node->value, "10");

    auto else_node = dynamic_cast<NumberLiteral*>(cond_expr->else_branch.get());
    ASSERT_NE(else_node, nullptr);
    EXPECT_EQ(else_node->value, "20");
}

TEST_F(AstBooleanAndConditionalsTest, ValidatesNestedConditionals) {
    /*
     Expected AST Shape:
     Conditional (Cond: x)
      |- Then: 1
      |- Else: Conditional (Cond: y)
                 |- Then: 2
                 |- Else: 3
    */

    auto ast = parse_code("let a = if x then 1 else if y then 2 else 3");
    auto root_expr = get_root_expr(ast);

    auto outer_cond = dynamic_cast<ConditionalExpression*>(root_expr);
    ASSERT_NE(outer_cond, nullptr);

    auto inner_cond = dynamic_cast<ConditionalExpression*>(outer_cond->else_branch.get());
    ASSERT_NE(inner_cond, nullptr);

    auto inner_cond_id = dynamic_cast<IdentifierAccess*>(inner_cond->condition.get());
    ASSERT_NE(inner_cond_id, nullptr);
    EXPECT_EQ(inner_cond_id->name, "y");

    auto inner_then = dynamic_cast<NumberLiteral*>(inner_cond->then_branch.get());
    ASSERT_NE(inner_then, nullptr);
    EXPECT_EQ(inner_then->value, "2");

    auto inner_else = dynamic_cast<NumberLiteral*>(inner_cond->else_branch.get());
    ASSERT_NE(inner_else, nullptr);
    EXPECT_EQ(inner_else->value, "3");
}

TEST_F(AstBooleanAndConditionalsTest, ValidatesFunctionCallsInAllBranches) {
    /*
     Expected AST Shape:
     Conditional
      |- Cond: FunctionCall (is_ready)
      |- Then: FunctionCall (start)
      |- Else: FunctionCall (stop)
    */

    auto ast = parse_code("let a = if is_ready() then start() else stop()");
    auto root_cond = dynamic_cast<ConditionalExpression*>(get_root_expr(ast));
    ASSERT_NE(root_cond, nullptr);

    // 1. Condition is a function call
    auto cond_call = dynamic_cast<FunctionCall*>(root_cond->condition.get());
    ASSERT_NE(cond_call, nullptr);
    auto cond_target = dynamic_cast<IdentifierAccess*>(cond_call->target.get());
    ASSERT_NE(cond_target, nullptr);
    EXPECT_EQ(cond_target->name, "is_ready");
    EXPECT_EQ(cond_call->arguments.size(), 0);

    // 2. Then branch is a function call
    auto then_call = dynamic_cast<FunctionCall*>(root_cond->then_branch.get());
    ASSERT_NE(then_call, nullptr);
    auto then_target = dynamic_cast<IdentifierAccess*>(then_call->target.get());
    ASSERT_NE(then_target, nullptr);
    EXPECT_EQ(then_target->name, "start");

    // 3. Else branch is a function call
    auto else_call = dynamic_cast<FunctionCall*>(root_cond->else_branch.get());
    ASSERT_NE(else_call, nullptr);
    auto else_target = dynamic_cast<IdentifierAccess*>(else_call->target.get());
    ASSERT_NE(else_target, nullptr);
    EXPECT_EQ(else_target->name, "stop");
}

TEST_F(AstBooleanAndConditionalsTest, ValidatesComparisonAgainstFunctionCallWithParams) {
    // Tests that a binary expression correctly wraps a function call with arguments
    // inside the condition slot.

    auto ast = parse_code("let a = if get_value(a: x, b: 1) == 100 then onSuccess() else onError()");
    auto root_cond = dynamic_cast<ConditionalExpression*>(get_root_expr(ast));
    ASSERT_NE(root_cond, nullptr);

    // 1. Condition is an Equality check
    auto cond_bin = dynamic_cast<BinaryExpression*>(root_cond->condition.get());
    ASSERT_NE(cond_bin, nullptr);
    EXPECT_EQ(cond_bin->op, TokenType::Equals);

    // 2. Left side of equality is a function call: get_value(x, 1)
    auto left_call = dynamic_cast<FunctionCall*>(cond_bin->left.get());
    ASSERT_NE(left_call, nullptr);
    auto left_target = dynamic_cast<IdentifierAccess*>(left_call->target.get());
    ASSERT_NE(left_target, nullptr);
    EXPECT_EQ(left_target->name, "get_value");

    // Verify arguments: 'x' and '1'
    ASSERT_EQ(left_call->arguments.size(), 2);
    auto arg_1 = dynamic_cast<IdentifierAccess*>(left_call->arguments[0].second.get());
    auto arg_2 = dynamic_cast<NumberLiteral*>(left_call->arguments[1].second.get());
    ASSERT_NE(arg_1, nullptr);
    ASSERT_NE(arg_2, nullptr);
    EXPECT_EQ(arg_1->name, "x");
    EXPECT_EQ(arg_2->value, "1");

    // 3. Right side of equality is NumberLiteral(100)
    auto right_num = dynamic_cast<NumberLiteral*>(cond_bin->right.get());
    ASSERT_NE(right_num, nullptr);
    EXPECT_EQ(right_num->value, "100");
}

TEST_F(AstBooleanAndConditionalsTest, ValidatesExtremeFunctionNesting) {
    // Tests that function parameters can safely be other function calls within conditional branches.

    auto ast = parse_code("let a = if check(c: fetch(u: url)) then process(p: transform(d: data)) else fallback(f: get_default())");
    auto root_cond = dynamic_cast<ConditionalExpression*>(get_root_expr(ast));
    ASSERT_NE(root_cond, nullptr);

    // 1. Condition: check(fetch(url))
    auto cond_call = dynamic_cast<FunctionCall*>(root_cond->condition.get());
    ASSERT_NE(cond_call, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(cond_call->target.get())->name, "check");
    ASSERT_EQ(cond_call->arguments.size(), 1);

    auto cond_arg_call = dynamic_cast<FunctionCall*>(cond_call->arguments[0].second.get());
    ASSERT_NE(cond_arg_call, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(cond_arg_call->target.get())->name, "fetch");

    // 2. Then Branch: process(transform(data))
    auto then_call = dynamic_cast<FunctionCall*>(root_cond->then_branch.get());
    ASSERT_NE(then_call, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(then_call->target.get())->name, "process");
    ASSERT_EQ(then_call->arguments.size(), 1);

    auto then_arg_call = dynamic_cast<FunctionCall*>(then_call->arguments[0].second.get());
    ASSERT_NE(then_arg_call, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(then_arg_call->target.get())->name, "transform");

    // 3. Else Branch: fallback(get_default())
    auto else_call = dynamic_cast<FunctionCall*>(root_cond->else_branch.get());
    ASSERT_NE(else_call, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(else_call->target.get())->name, "fallback");
    ASSERT_EQ(else_call->arguments.size(), 1);

    auto else_arg_call = dynamic_cast<FunctionCall*>(else_call->arguments[0].second.get());
    ASSERT_NE(else_arg_call, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(else_arg_call->target.get())->name, "get_default");
    EXPECT_EQ(else_arg_call->arguments.size(), 0); // get_default takes no args
}