#include <gtest/gtest.h>
#include "stages/parser/parser_stage.h"
#include "stages/lexer/lexer_stage.h"
#include "stages/parser/ast.h"

using namespace valuascript;
using namespace valuascript::compiler;

class AstAssignmentTest : public testing::Test {
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

    Assignment* get_first_assignment(const std::shared_ptr<Program>& ast) {
        if (ast->execution_steps.empty()) return nullptr;
        return dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    }
};

TEST_F(AstAssignmentTest, ValidatesNestedFunctionCallsOnRHS) {
    // Code: let result = outer(inner(x), middle(y, z))

    auto ast = parse_code("let result = outer(inner(x), middle(y, z))");
    auto assign_node = get_first_assignment(ast);
    ASSERT_NE(assign_node, nullptr);

    // 1. Check Target
    ASSERT_EQ(assign_node->targets.size(), 1);
    EXPECT_EQ(assign_node->targets[0], "result");

    // 2. Check Outer Call
    auto outer_call = dynamic_cast<FunctionCall*>(assign_node->value.get());
    ASSERT_NE(outer_call, nullptr);
    auto outer_target = dynamic_cast<IdentifierAccess*>(outer_call->target.get());
    ASSERT_NE(outer_target, nullptr);
    EXPECT_EQ(outer_target->name, "outer");
    ASSERT_EQ(outer_call->arguments.size(), 2);

    // 3. Check First Argument (inner(x))
    auto arg1_call = dynamic_cast<FunctionCall*>(outer_call->arguments[0].get());
    ASSERT_NE(arg1_call, nullptr);
    auto arg1_target = dynamic_cast<IdentifierAccess*>(arg1_call->target.get());
    ASSERT_NE(arg1_target, nullptr);
    EXPECT_EQ(arg1_target->name, "inner");

    auto inner_arg = dynamic_cast<IdentifierAccess*>(arg1_call->arguments[0].get());
    ASSERT_NE(inner_arg, nullptr);
    EXPECT_EQ(inner_arg->name, "x");

    // 4. Check Second Argument (middle(y, z))
    auto arg2_call = dynamic_cast<FunctionCall*>(outer_call->arguments[1].get());
    ASSERT_NE(arg2_call, nullptr);
    auto arg2_target = dynamic_cast<IdentifierAccess*>(arg2_call->target.get());
    ASSERT_NE(arg2_target, nullptr);
    EXPECT_EQ(arg2_target->name, "middle");
    ASSERT_EQ(arg2_call->arguments.size(), 2);

    auto middle_arg1 = dynamic_cast<IdentifierAccess*>(arg2_call->arguments[0].get());
    auto middle_arg2 = dynamic_cast<IdentifierAccess*>(arg2_call->arguments[1].get());
    ASSERT_NE(middle_arg1, nullptr);
    ASSERT_NE(middle_arg2, nullptr);
    EXPECT_EQ(middle_arg1->name, "y");
    EXPECT_EQ(middle_arg2->name, "z");
}

TEST_F(AstAssignmentTest, ValidatesConditionalExpressionAsFunctionParameter) {
    // Code: let a = process(if x > 0 then 1 else -1)
    // Proves that standard expressions can wrap conditionals cleanly inside argument lists.

    auto ast = parse_code("let a = process(if x > 0 then 1 else -1)");
    auto assign_node = get_first_assignment(ast);
    ASSERT_NE(assign_node, nullptr);

    auto process_call = dynamic_cast<FunctionCall*>(assign_node->value.get());
    ASSERT_NE(process_call, nullptr);
    ASSERT_EQ(process_call->arguments.size(), 1);

    // The argument must be a ConditionalExpression
    auto cond_arg = dynamic_cast<ConditionalExpression*>(process_call->arguments[0].get());
    ASSERT_NE(cond_arg, nullptr) << "Argument was not correctly parsed as a conditional expression.";

    // Check Condition: x > 0
    auto condition = dynamic_cast<BinaryExpression*>(cond_arg->condition.get());
    ASSERT_NE(condition, nullptr);
    EXPECT_EQ(condition->op, TokenType::Greater);

    // Check Then Branch: 1
    auto then_branch = dynamic_cast<NumberLiteral*>(cond_arg->then_branch.get());
    ASSERT_NE(then_branch, nullptr);
    EXPECT_EQ(then_branch->value, "1");

    // Check Else Branch: -1
    auto else_branch = dynamic_cast<UnaryExpression*>(cond_arg->else_branch.get());
    ASSERT_NE(else_branch, nullptr);
    EXPECT_EQ(else_branch->op, TokenType::Minus);
}

TEST_F(AstAssignmentTest, ValidatesMultipleAssignmentTargets) {
    // Code: let x, y = get_coordinates()
    // Proves the AST correctly maps a comma-separated list of identifiers into the assignment targets.

    auto ast = parse_code("let x, y = get_coordinates()");
    auto assign_node = get_first_assignment(ast);
    ASSERT_NE(assign_node, nullptr);

    // Check Targets
    ASSERT_EQ(assign_node->targets.size(), 2);
    EXPECT_EQ(assign_node->targets[0], "x");
    EXPECT_EQ(assign_node->targets[1], "y");

    // Check Value
    auto call_val = dynamic_cast<FunctionCall*>(assign_node->value.get());
    ASSERT_NE(call_val, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(call_val->target.get())->name, "get_coordinates");
}

TEST_F(AstAssignmentTest, ValidatesComplexChainedPostfixOnRHS) {
    // Code: let data = get_matrix()[0](arg)
    // Evaluates a function call, accesses the 0th index, and calls the result as a function.

    auto ast = parse_code("let data = get_matrix()[0](arg)");
    auto assign_node = get_first_assignment(ast);
    ASSERT_NE(assign_node, nullptr);

    // Root value is the final function invocation: (...)(arg)
    auto final_call = dynamic_cast<FunctionCall*>(assign_node->value.get());
    ASSERT_NE(final_call, nullptr);
    ASSERT_EQ(final_call->arguments.size(), 1);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(final_call->arguments[0].get())->name, "arg");

    // Target of the final call is the vector access: (...)[0]
    auto vector_access = dynamic_cast<VectorAccess*>(final_call->target.get());
    ASSERT_NE(vector_access, nullptr);
    auto index_val = dynamic_cast<NumberLiteral*>(vector_access->index.get());
    ASSERT_NE(index_val, nullptr);
    EXPECT_EQ(index_val->value, "0");

    // Target of the vector access is the initial function call: get_matrix()
    auto initial_call = dynamic_cast<FunctionCall*>(vector_access->target.get());
    ASSERT_NE(initial_call, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(initial_call->target.get())->name, "get_matrix");
}

TEST_F(AstAssignmentTest, ValidatesPrimitiveLiteralsAsParameters) {
    // Code: let a = test_primitives(42, "hello", true, false)
    // Proves the parameter list correctly parses all base literal types.

    auto ast = parse_code("let a = test_primitives(42, \"hello\", true, false)");
    auto assign_node = get_first_assignment(ast);
    ASSERT_NE(assign_node, nullptr);

    auto func_call = dynamic_cast<FunctionCall*>(assign_node->value.get());
    ASSERT_NE(func_call, nullptr);
    ASSERT_EQ(func_call->arguments.size(), 4);

    // Arg 0: Number
    auto arg_num = dynamic_cast<NumberLiteral*>(func_call->arguments[0].get());
    ASSERT_NE(arg_num, nullptr);
    EXPECT_EQ(arg_num->value, "42");

    // Arg 1: String (remember quotes are preserved by the Lexer!)
    auto arg_str = dynamic_cast<StringLiteral*>(func_call->arguments[1].get());
    ASSERT_NE(arg_str, nullptr);
    EXPECT_EQ(arg_str->value, "\"hello\"");

    // Arg 2: Boolean True
    auto arg_bool_t = dynamic_cast<BooleanLiteral*>(func_call->arguments[2].get());
    ASSERT_NE(arg_bool_t, nullptr);
    EXPECT_EQ(arg_bool_t->value, true);

    // Arg 3: Boolean False
    auto arg_bool_f = dynamic_cast<BooleanLiteral*>(func_call->arguments[3].get());
    ASSERT_NE(arg_bool_f, nullptr);
    EXPECT_EQ(arg_bool_f->value, false);
}

TEST_F(AstAssignmentTest, ValidatesMathAndLogicExpressionsAsParameters) {
    // Code: let a = calculate(x + 2 * 3, not y and z)
    // Proves that commas inside parameter lists do not break standard operator precedence.

    auto ast = parse_code("let a = calculate(x + 2 * 3, not y and z)");
    auto func_call = dynamic_cast<FunctionCall*>(get_first_assignment(ast)->value.get());
    ASSERT_NE(func_call, nullptr);
    ASSERT_EQ(func_call->arguments.size(), 2);

    // --- Arg 0: x + 2 * 3 ---
    auto arg_math = dynamic_cast<BinaryExpression*>(func_call->arguments[0].get());
    ASSERT_NE(arg_math, nullptr) << "First argument should be a BinaryExpression (+)";
    EXPECT_EQ(arg_math->op, TokenType::Plus);

    auto math_left = dynamic_cast<IdentifierAccess*>(arg_math->left.get());
    ASSERT_NE(math_left, nullptr);
    EXPECT_EQ(math_left->name, "x");

    auto math_right = dynamic_cast<BinaryExpression*>(arg_math->right.get());
    ASSERT_NE(math_right, nullptr);
    EXPECT_EQ(math_right->op, TokenType::Star); // Proves precedence holds inside args

    // --- Arg 1: not y and z ---
    auto arg_logic = dynamic_cast<BinaryExpression*>(func_call->arguments[1].get());
    ASSERT_NE(arg_logic, nullptr) << "Second argument should be a BinaryExpression (and)";
    EXPECT_EQ(arg_logic->op, TokenType::And);

    auto logic_left = dynamic_cast<UnaryExpression*>(arg_logic->left.get());
    ASSERT_NE(logic_left, nullptr);
    EXPECT_EQ(logic_left->op, TokenType::Not); // Proves unary precedence holds inside args

    auto logic_right = dynamic_cast<IdentifierAccess*>(arg_logic->right.get());
    ASSERT_NE(logic_right, nullptr);
    EXPECT_EQ(logic_right->name, "z");
}

TEST_F(AstAssignmentTest, ValidatesPostfixExpressionsAsParameters) {
    // Code: let a = process(matrix[0][1], fetch_data())
    // Proves vector access and nested function calls cleanly separate by comma.

    auto ast = parse_code("let a = process(matrix[0][1], fetch_data())");
    auto func_call = dynamic_cast<FunctionCall*>(get_first_assignment(ast)->value.get());
    ASSERT_NE(func_call, nullptr);
    ASSERT_EQ(func_call->arguments.size(), 2);

    // --- Arg 0: matrix[0][1] ---
    auto arg_vec = dynamic_cast<VectorAccess*>(func_call->arguments[0].get());
    ASSERT_NE(arg_vec, nullptr);

    auto inner_vec = dynamic_cast<VectorAccess*>(arg_vec->target.get());
    ASSERT_NE(inner_vec, nullptr);
    auto vec_id = dynamic_cast<IdentifierAccess*>(inner_vec->target.get());
    ASSERT_NE(vec_id, nullptr);
    EXPECT_EQ(vec_id->name, "matrix");

    // --- Arg 1: fetch_data() ---
    auto arg_call = dynamic_cast<FunctionCall*>(func_call->arguments[1].get());
    ASSERT_NE(arg_call, nullptr);
    auto call_id = dynamic_cast<IdentifierAccess*>(arg_call->target.get());
    ASSERT_NE(call_id, nullptr);
    EXPECT_EQ(call_id->name, "fetch_data");
    EXPECT_EQ(arg_call->arguments.size(), 0);
}

TEST_F(AstAssignmentTest, ValidatesParenthesizedExpressionsAsParameters) {
    // Code: let a = transform((x + y) * z)
    // Proves that grouping parentheses inside argument lists don't conflict with function call parentheses.

    auto ast = parse_code("let a = transform((x + y) * z)");
    auto func_call = dynamic_cast<FunctionCall*>(get_first_assignment(ast)->value.get());
    ASSERT_NE(func_call, nullptr);
    ASSERT_EQ(func_call->arguments.size(), 1);

    // The argument should be the multiplication, with the addition on the left branch
    auto mult_node = dynamic_cast<BinaryExpression*>(func_call->arguments[0].get());
    ASSERT_NE(mult_node, nullptr);
    EXPECT_EQ(mult_node->op, TokenType::Star);

    auto add_node = dynamic_cast<BinaryExpression*>(mult_node->left.get());
    ASSERT_NE(add_node, nullptr);
    EXPECT_EQ(add_node->op, TokenType::Plus);
}