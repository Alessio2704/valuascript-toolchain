#include "frontend/parser/ast_base_test.h"

namespace valuascript::compiler::test {
    TEST_F(AstBaseTest, ValidatesDeepReassignmentGeometry) {
        // Proves the target is correctly parsed as: BracketAccess -> DotAccess -> Identifier

        auto ast = parse_code("portfolio.assets[0] = 100");
        ASSERT_EQ(ast->execution_steps.size(), 1);

        auto reassignment = dynamic_cast<Reassignment *>(ast->execution_steps[0].get());
        ASSERT_NE(reassignment, nullptr) << "Expected a Reassignment node.";

        // 1. Verify the Right-Hand Side (Value)
        auto value = dynamic_cast<NumberLiteral *>(reassignment->value.get());
        ASSERT_NE(value, nullptr);
        EXPECT_EQ(value->value, "100");

        // 2. Verify the Left-Hand Side (Target)
        auto target_bracket = dynamic_cast<BracketAccess *>(reassignment->target.get());
        ASSERT_NE(target_bracket, nullptr) << "Target root should be a BracketAccess";

        // Check the index
        auto index_val = dynamic_cast<NumberLiteral *>(target_bracket->index.get());
        ASSERT_NE(index_val, nullptr);
        EXPECT_EQ(index_val->value, "0");

        // Check the target of the bracket access
        auto target_dot = dynamic_cast<DotAccess *>(target_bracket->target.get());
        ASSERT_NE(target_dot, nullptr) << "Bracket target should be a DotAccess";
        EXPECT_EQ(target_dot->property_name, "assets");

        // Check the target of the dot access
        auto target_id = dynamic_cast<IdentifierAccess *>(target_dot->target.get());
        ASSERT_NE(target_id, nullptr) << "Dot target should be an IdentifierAccess";
        EXPECT_EQ(target_id->name, "portfolio");
    }

    TEST_F(AstBaseTest, ValidatesExpressionStatementGeometry) {
        auto ast = parse_code("sys.init()");
        ASSERT_EQ(ast->execution_steps.size(), 1);

        auto expr_stmt = dynamic_cast<ExpressionStatement *>(ast->execution_steps[0].get());
        ASSERT_NE(expr_stmt, nullptr) << "Expected an ExpressionStatement node.";

        auto func_call = dynamic_cast<FunctionCall *>(expr_stmt->expr.get());
        ASSERT_NE(func_call, nullptr) << "Expected the wrapped expression to be a FunctionCall.";

        auto dot_access = dynamic_cast<DotAccess *>(func_call->target.get());
        ASSERT_NE(dot_access, nullptr);
        EXPECT_EQ(dot_access->property_name, "init");

        auto target_id = dynamic_cast<IdentifierAccess *>(dot_access->target.get());
        ASSERT_NE(target_id, nullptr);
        EXPECT_EQ(target_id->name, "sys");
    }

    TEST_F(AstBaseTest, ValidatesMultiDimensionalBracketReassignment) {
        auto ast = parse_code("matrix[0][1] = 42");
        ASSERT_EQ(ast->execution_steps.size(), 1);

        auto reassignment = dynamic_cast<Reassignment *>(ast->execution_steps[0].get());
        ASSERT_NE(reassignment, nullptr);

        // Check RHS
        auto rhs_val = dynamic_cast<NumberLiteral *>(reassignment->value.get());
        ASSERT_NE(rhs_val, nullptr);
        EXPECT_EQ(rhs_val->value, "42");

        // Check LHS: Outer BracketAccess (the [1] part)
        auto outer_bracket = dynamic_cast<BracketAccess *>(reassignment->target.get());
        ASSERT_NE(outer_bracket, nullptr) << "Expected outer target to be a BracketAccess";

        auto outer_index = dynamic_cast<NumberLiteral *>(outer_bracket->index.get());
        ASSERT_NE(outer_index, nullptr);
        EXPECT_EQ(outer_index->value, "1");

        // Check LHS: Inner BracketAccess (the matrix[0] part)
        auto inner_bracket = dynamic_cast<BracketAccess *>(outer_bracket->target.get());
        ASSERT_NE(inner_bracket, nullptr) << "Expected inner target to be a BracketAccess";

        auto inner_index = dynamic_cast<NumberLiteral *>(inner_bracket->index.get());
        ASSERT_NE(inner_index, nullptr);
        EXPECT_EQ(inner_index->value, "0");

        auto root_id = dynamic_cast<IdentifierAccess *>(inner_bracket->target.get());
        ASSERT_NE(root_id, nullptr);
        EXPECT_EQ(root_id->name, "matrix");
    }

    TEST_F(AstBaseTest, ValidatesReassignmentToFunctionCallProperty) {
        auto ast = parse_code("get_config().threshold = 10.5");
        ASSERT_EQ(ast->execution_steps.size(), 1);

        auto reassignment = dynamic_cast<Reassignment *>(ast->execution_steps[0].get());
        ASSERT_NE(reassignment, nullptr);

        // Check LHS: DotAccess
        auto dot_access = dynamic_cast<DotAccess *>(reassignment->target.get());
        ASSERT_NE(dot_access, nullptr) << "Expected target to be a DotAccess";
        EXPECT_EQ(dot_access->property_name, "threshold");

        // Check Target of DotAccess: FunctionCall
        auto func_call = dynamic_cast<FunctionCall *>(dot_access->target.get());
        ASSERT_NE(func_call, nullptr) << "Expected the target of the dot access to be a FunctionCall";

        auto func_id = dynamic_cast<IdentifierAccess *>(func_call->target.get());
        ASSERT_NE(func_id, nullptr);
        EXPECT_EQ(func_id->name, "get_config");
        EXPECT_EQ(func_call->arguments.size(), 0);
    }

    TEST_F(AstBaseTest, ValidatesDeepChainedMethodCallExpressionStatement) {
        auto ast = parse_code("system.modules[0].initialize(s: true)");
        ASSERT_EQ(ast->execution_steps.size(), 1);

        auto expr_stmt = dynamic_cast<ExpressionStatement *>(ast->execution_steps[0].get());
        ASSERT_NE(expr_stmt, nullptr);

        // 1. Root is a FunctionCall
        auto func_call = dynamic_cast<FunctionCall *>(expr_stmt->expr.get());
        ASSERT_NE(func_call, nullptr) << "Expected expression statement root to be a FunctionCall";
        ASSERT_EQ(func_call->arguments.size(), 1);

        auto arg_val = dynamic_cast<BooleanLiteral *>(func_call->arguments[0].second.get());
        ASSERT_NE(arg_val, nullptr);
        EXPECT_TRUE(arg_val->value);

        // 2. Target of FunctionCall is a DotAccess (.initialize)
        auto dot_init = dynamic_cast<DotAccess *>(func_call->target.get());
        ASSERT_NE(dot_init, nullptr);
        EXPECT_EQ(dot_init->property_name, "initialize");

        // 3. Target of DotAccess is a BracketAccess ([0])
        auto bracket_acc = dynamic_cast<BracketAccess *>(dot_init->target.get());
        ASSERT_NE(bracket_acc, nullptr);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(bracket_acc->index.get())->value, "0");

        // 4. Target of BracketAccess is a DotAccess (.modules)
        auto dot_modules = dynamic_cast<DotAccess *>(bracket_acc->target.get());
        ASSERT_NE(dot_modules, nullptr);
        EXPECT_EQ(dot_modules->property_name, "modules");

        // 5. Target is Identifier (system)
        auto root_id = dynamic_cast<IdentifierAccess *>(dot_modules->target.get());
        ASSERT_NE(root_id, nullptr);
        EXPECT_EQ(root_id->name, "system");
    }

    TEST_F(AstBaseTest, ValidatesSwitchExpressionAsReassignmentValue) {
        auto ast = parse_code("state.status = switch (res) { case UP -> 1 default -> 0 }");
        ASSERT_EQ(ast->execution_steps.size(), 1);

        auto reassignment = dynamic_cast<Reassignment *>(ast->execution_steps[0].get());
        ASSERT_NE(reassignment, nullptr);

        // LHS: state.status
        auto dot_access = dynamic_cast<DotAccess *>(reassignment->target.get());
        ASSERT_NE(dot_access, nullptr);
        EXPECT_EQ(dot_access->property_name, "status");

        auto target_id = dynamic_cast<IdentifierAccess *>(dot_access->target.get());
        ASSERT_NE(target_id, nullptr);
        EXPECT_EQ(target_id->name, "state");

        // RHS: SwitchExpression
        auto switch_expr = dynamic_cast<SwitchExpression *>(reassignment->value.get());
        ASSERT_NE(switch_expr, nullptr) << "Expected RHS to be a SwitchExpression";

        auto switch_target = dynamic_cast<IdentifierAccess *>(switch_expr->target.get());
        ASSERT_NE(switch_target, nullptr);
        EXPECT_EQ(switch_target->name, "res");

        ASSERT_EQ(switch_expr->cases.size(), 1);
        EXPECT_EQ(switch_expr->cases[0].first[0], "UP");

        ASSERT_NE(switch_expr->default_case, nullptr);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(switch_expr->default_case.get())->value, "0");
    }

    TEST_F(AstBaseTest, ValidatesReassignmentToSelfProperty) {
        // Proves that TokenTraits::is_valid_lvalue() implicitly allows DotAccess targets
        // that are based on 'self', allowing statements like `self.val = 10`.
        // (Note: Semantic validation of WHERE this happens is deferred to the semantic analyzer).

        auto ast = parse_code("self.counter = self.counter + 1");

        ASSERT_EQ(ast->execution_steps.size(), 1);
        auto reassignment = dynamic_cast<Reassignment *>(ast->execution_steps[0].get());
        ASSERT_NE(reassignment, nullptr) << "Top-level statement must be parsed as a Reassignment";

        // Target: self.counter
        auto target_dot = dynamic_cast<DotAccess *>(reassignment->target.get());
        ASSERT_NE(target_dot, nullptr) << "Reassignment target must be a DotAccess";
        EXPECT_EQ(target_dot->property_name, "counter");
        ASSERT_NE(dynamic_cast<SelfExpression *>(target_dot->target.get()), nullptr);

        // Value: self.counter + 1
        auto bin_expr = dynamic_cast<BinaryExpression *>(reassignment->value.get());
        ASSERT_NE(bin_expr, nullptr);
        EXPECT_EQ(bin_expr->op, TokenType::Plus);

        auto left_dot = dynamic_cast<DotAccess *>(bin_expr->left.get());
        ASSERT_NE(left_dot, nullptr);
        EXPECT_EQ(left_dot->property_name, "counter");
        ASSERT_NE(dynamic_cast<SelfExpression *>(left_dot->target.get()), nullptr);

        auto right_num = dynamic_cast<NumberLiteral *>(bin_expr->right.get());
        ASSERT_NE(right_num, nullptr);
        EXPECT_EQ(right_num->value, "1");
    }
}
