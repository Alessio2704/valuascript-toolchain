#include <gtest/gtest.h>
#include "stages/parser/parser_stage.h"
#include "stages/lexer/lexer_stage.h"
#include "stages/parser/ast.h"

using namespace valuascript;
using namespace valuascript::compiler;

class AstFunctionDefinitionTest : public testing::Test {
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

    // Helper to extract the first function definition
    FunctionDefinition* get_first_func(const std::shared_ptr<Program>& ast) {
        if (ast->function_definitions.empty()) return nullptr;
        return ast->function_definitions[0].get();
    }
};

TEST_F(AstFunctionDefinitionTest, ValidatesSignatureAndDocstring) {
    // Code: func calculate(x: scalar, y: boolean) -> (scalar, scalar) { """Computes values""" }

    auto ast = parse_code(R"(func calculate(x: scalar, y: boolean) -> (scalar, scalar) { """Computes values""" })");
    auto func = get_first_func(ast);
    ASSERT_NE(func, nullptr);

    // 1. Function Name
    EXPECT_EQ(func->name, "calculate");

    // 2. Docstring
    ASSERT_TRUE(func->docstring.has_value());
    EXPECT_EQ(func->docstring.value(), "\"\"\"Computes values\"\"\"");

    // 3. Parameters
    ASSERT_EQ(func->parameters.size(), 2);

    EXPECT_EQ(func->parameters[0].name, "x");
    EXPECT_EQ(func->parameters[0].type->name, "scalar");
    EXPECT_TRUE(func->parameters[0].type->generic_args.empty());

    EXPECT_EQ(func->parameters[1].name, "y");
    EXPECT_EQ(func->parameters[1].type->name, "boolean");

    // 4. Return Types (Tuple)
    ASSERT_EQ(func->return_types.size(), 2);
    EXPECT_EQ(func->return_types[0]->name, "scalar");
    EXPECT_EQ(func->return_types[1]->name, "scalar");

    // Body should be empty (docstring is not a statement)
    EXPECT_EQ(func->body.size(), 0);
}

TEST_F(AstFunctionDefinitionTest, ValidatesDeeplyNestedGenerics) {
    // Code: func process(data: map<string, vector<vector<scalar>>>) -> scalar {}
    // Tests that TypeAnnotation nodes nest infinitely without dropping context.

    auto ast = parse_code("func process(data: map<string, vector<vector<scalar>>>) -> scalar {}");
    auto func = get_first_func(ast);
    ASSERT_NE(func, nullptr);

    ASSERT_EQ(func->parameters.size(), 1);
    auto root_type = func->parameters[0].type.get();

    // Outer type: map
    EXPECT_EQ(root_type->name, "map");
    ASSERT_EQ(root_type->generic_args.size(), 2);

    // First generic arg: string
    EXPECT_EQ(root_type->generic_args[0]->name, "string");

    // Second generic arg: vector
    auto level1_vector = root_type->generic_args[1].get();
    EXPECT_EQ(level1_vector->name, "vector");
    ASSERT_EQ(level1_vector->generic_args.size(), 1);

    // Third generic arg (nested): vector
    auto level2_vector = level1_vector->generic_args[0].get();
    EXPECT_EQ(level2_vector->name, "vector");
    ASSERT_EQ(level2_vector->generic_args.size(), 1);

    // Deepest generic arg: scalar
    auto deepest_scalar = level2_vector->generic_args[0].get();
    EXPECT_EQ(deepest_scalar->name, "scalar");
    EXPECT_TRUE(deepest_scalar->generic_args.empty());
}

TEST_F(AstFunctionDefinitionTest, ValidatesBodyStatementsExecutionOrder) {
    // Code:
    // func compute() -> scalar {
    //     let a = 10
    //     let b = a * 2
    //     return b
    // }

    auto ast = parse_code("func compute() -> scalar { let a = 10 \n let b = a * 2 \n return b }");
    auto func = get_first_func(ast);
    ASSERT_NE(func, nullptr);

    ASSERT_EQ(func->body.size(), 3) << "Expected exactly 3 statements in function body.";

    // Statement 1: let a = 10
    auto stmt_1 = dynamic_cast<Assignment*>(func->body[0].get());
    ASSERT_NE(stmt_1, nullptr);
    ASSERT_EQ(stmt_1->targets.size(), 1);
    EXPECT_EQ(stmt_1->targets[0], "a");
    auto val_1 = dynamic_cast<NumberLiteral*>(stmt_1->value.get());
    ASSERT_NE(val_1, nullptr);
    EXPECT_EQ(val_1->value, "10");

    // Statement 2: let b = a * 2
    auto stmt_2 = dynamic_cast<Assignment*>(func->body[1].get());
    ASSERT_NE(stmt_2, nullptr);
    ASSERT_EQ(stmt_2->targets.size(), 1);
    EXPECT_EQ(stmt_2->targets[0], "b");
    auto val_2 = dynamic_cast<BinaryExpression*>(stmt_2->value.get());
    ASSERT_NE(val_2, nullptr);
    EXPECT_EQ(val_2->op, TokenType::Star);

    // Statement 3: return b
    auto stmt_3 = dynamic_cast<ReturnStatement*>(func->body[2].get());
    ASSERT_NE(stmt_3, nullptr);
    auto ret_1 = dynamic_cast<IdentifierAccess*>(stmt_3->values[0].get());
    ASSERT_NE(ret_1, nullptr);
    EXPECT_EQ(ret_1->name, "b");
}

TEST_F(AstFunctionDefinitionTest, ValidatesTupleReturnStatement) {
    // Code: func bounds() -> (scalar, scalar) { return 10, 20 }

    auto ast = parse_code("func bounds() -> (scalar, scalar) { return 10, 20 }");
    auto func = get_first_func(ast);
    ASSERT_NE(func, nullptr);

    ASSERT_EQ(func->body.size(), 1);
    auto return_stmt = dynamic_cast<ReturnStatement*>(func->body[0].get());
    ASSERT_NE(return_stmt, nullptr);

    // Validate that the ReturnStatement captured both values natively
    ASSERT_EQ(return_stmt->values.size(), 2);

    auto ret_1 = dynamic_cast<NumberLiteral*>(return_stmt->values[0].get());
    auto ret_2 = dynamic_cast<NumberLiteral*>(return_stmt->values[1].get());

    ASSERT_NE(ret_1, nullptr);
    ASSERT_NE(ret_2, nullptr);
    EXPECT_EQ(ret_1->value, "10");
    EXPECT_EQ(ret_2->value, "20");
}

TEST_F(AstFunctionDefinitionTest, ValidatesBodyWithNestedCallsAndSignatureParams) {
    // Code:
    // func compute(x: scalar, y: scalar) -> scalar {
    //     let temp = add(x, multiply(y, 2))
    //     return temp
    // }
    // Tests that function parameters (x, y) are correctly parsed as IdentifierAccess
    // inside deeply nested function calls on the right-hand side of an assignment.

    auto ast = parse_code("func compute(x: scalar, y: scalar) -> scalar { let temp = add(a: x, b: multiply(c: y, d: 2)) \n return temp }");
    auto func = get_first_func(ast);
    ASSERT_NE(func, nullptr);

    ASSERT_EQ(func->body.size(), 2) << "Expected 2 statements: let and return.";

    // --- Statement 1: let temp = add(...) ---
    auto assign_stmt = dynamic_cast<Assignment*>(func->body[0].get());
    ASSERT_NE(assign_stmt, nullptr);
    EXPECT_EQ(assign_stmt->targets[0], "temp");

    // The value is the outer function call: add(...)
    auto add_call = dynamic_cast<FunctionCall*>(assign_stmt->value.get());
    ASSERT_NE(add_call, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(add_call->target.get())->name, "add");
    ASSERT_EQ(add_call->arguments.size(), 2);

    // Argument 1 of add(): x (Original signature param)
    auto arg_x = dynamic_cast<IdentifierAccess*>(add_call->arguments[0].second.get());
    ASSERT_NE(arg_x, nullptr);
    EXPECT_EQ(arg_x->name, "x");

    // Argument 2 of add(): multiply(y, 2)
    auto mult_call = dynamic_cast<FunctionCall*>(add_call->arguments[1].second.get());
    ASSERT_NE(mult_call, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(mult_call->target.get())->name, "multiply");
    ASSERT_EQ(mult_call->arguments.size(), 2);

    // Argument 1 of multiply(): y (Original signature param)
    auto arg_y = dynamic_cast<IdentifierAccess*>(mult_call->arguments[0].second.get());
    ASSERT_NE(arg_y, nullptr);
    EXPECT_EQ(arg_y->name, "y");

    // Argument 2 of multiply(): 2
    auto arg_2 = dynamic_cast<NumberLiteral*>(mult_call->arguments[1].second.get());
    ASSERT_NE(arg_2, nullptr);
    EXPECT_EQ(arg_2->value, "2");
}

TEST_F(AstFunctionDefinitionTest, ValidatesConditionalInsideFunctionBody) {
    // Code:
    // func max(a: scalar, b: scalar) -> scalar {
    //     let res = if a > b then a else b
    //     return res
    // }
    // Tests that conditional expressions cleanly fit inside block statement assignments.

    auto ast = parse_code("func max(a: scalar, b: scalar) -> scalar { let res = if a > b then a else b \n return res }");
    auto func = get_first_func(ast);
    ASSERT_NE(func, nullptr);

    ASSERT_EQ(func->body.size(), 2);

    // --- Statement 1: let res = if ... ---
    auto assign_stmt = dynamic_cast<Assignment*>(func->body[0].get());
    ASSERT_NE(assign_stmt, nullptr);
    EXPECT_EQ(assign_stmt->targets[0], "res");

    auto cond_expr = dynamic_cast<ConditionalExpression*>(assign_stmt->value.get());
    ASSERT_NE(cond_expr, nullptr) << "Assigned value must be a ConditionalExpression.";

    // Condition: a > b
    auto condition = dynamic_cast<BinaryExpression*>(cond_expr->condition.get());
    ASSERT_NE(condition, nullptr);
    EXPECT_EQ(condition->op, TokenType::Greater);

    auto cond_left = dynamic_cast<IdentifierAccess*>(condition->left.get());
    auto cond_right = dynamic_cast<IdentifierAccess*>(condition->right.get());
    ASSERT_NE(cond_left, nullptr);
    ASSERT_NE(cond_right, nullptr);
    EXPECT_EQ(cond_left->name, "a");
    EXPECT_EQ(cond_right->name, "b");

    // Then branch: a
    auto then_branch = dynamic_cast<IdentifierAccess*>(cond_expr->then_branch.get());
    ASSERT_NE(then_branch, nullptr);
    EXPECT_EQ(then_branch->name, "a");

    // Else branch: b
    auto else_branch = dynamic_cast<IdentifierAccess*>(cond_expr->else_branch.get());
    ASSERT_NE(else_branch, nullptr);
    EXPECT_EQ(else_branch->name, "b");

    // --- Statement 2: return res ---
    auto return_stmt = dynamic_cast<ReturnStatement*>(func->body[1].get());
    ASSERT_NE(return_stmt, nullptr);
    ASSERT_EQ(return_stmt->values.size(), 1);

    auto ret_val = dynamic_cast<IdentifierAccess*>(return_stmt->values[0].get());
    ASSERT_NE(ret_val, nullptr);
    EXPECT_EQ(ret_val->name, "res");
}