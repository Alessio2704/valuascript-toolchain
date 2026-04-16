#include "frontend/parser/helpers/ast_base_test.h"

namespace valuascript::compiler::test {
    TEST_F(AstBaseTest, ValidatesStandardSwitchGeometry) {
        auto ast = parse_code("let val = switch (state) { case UP, SUS -> 10 case DOWN -> 20 default -> 30 }");
        ASSERT_EQ(ast->execution_steps.size(), 1);

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);

        auto switch_expr = dynamic_cast<SwitchExpression *>(assignment->value.get());
        ASSERT_NE(switch_expr, nullptr);

        // Target
        auto target_id = dynamic_cast<IdentifierAccess *>(switch_expr->target.get());
        ASSERT_NE(target_id, nullptr);
        EXPECT_EQ(target_id->name, "state");

        // Cases Vector
        ASSERT_EQ(switch_expr->cases.size(), 2);

        // Case 0: UP, SUS -> 10
        EXPECT_EQ(switch_expr->cases[0].first.size(), 2);
        EXPECT_EQ(switch_expr->cases[0].first[0], "UP");
        EXPECT_EQ(switch_expr->cases[0].first[1], "SUS");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(switch_expr->cases[0].second.get())->value, "10");

        // Case 1: DOWN -> 20
        EXPECT_EQ(switch_expr->cases[1].first.size(), 1);
        EXPECT_EQ(switch_expr->cases[1].first[0], "DOWN");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(switch_expr->cases[1].second.get())->value, "20");

        // Default
        ASSERT_NE(switch_expr->default_case, nullptr);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(switch_expr->default_case.get())->value, "30");
    }

    TEST_F(AstBaseTest, ValidatesSwitchWithNoDefault) {
        auto ast = parse_code("let val = switch (state) { case OPEN -> 1 }");
        ASSERT_EQ(ast->execution_steps.size(), 1);

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);

        auto switch_expr = dynamic_cast<SwitchExpression *>(assignment->value.get());
        ASSERT_NE(switch_expr, nullptr);

        ASSERT_EQ(switch_expr->cases.size(), 1);
        EXPECT_EQ(switch_expr->cases[0].first[0], "OPEN");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(switch_expr->cases[0].second.get())->value, "1");

        // The critical assertion: Default must be safely null
        EXPECT_EQ(switch_expr->default_case, nullptr);
    }

    TEST_F(AstBaseTest, ValidatesSwitchWithComplexExpressions) {
        auto ast = parse_code("let val = switch (sys.get_state()) { case ACTIVE -> base * 2 default -> 0 }");

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);
        auto switch_expr = dynamic_cast<SwitchExpression *>(assignment->value.get());
        ASSERT_NE(switch_expr, nullptr);

        // Verify complex target: sys.get_state()
        auto target_call = dynamic_cast<FunctionCall *>(switch_expr->target.get());
        ASSERT_NE(target_call, nullptr);
        auto target_dot = dynamic_cast<DotAccess *>(target_call->target.get());
        ASSERT_NE(target_dot, nullptr);
        EXPECT_EQ(target_dot->property_name, "get_state");

        // Verify complex result: base * 2
        auto result_bin = dynamic_cast<BinaryExpression *>(switch_expr->cases[0].second.get());
        ASSERT_NE(result_bin, nullptr);
        EXPECT_EQ(result_bin->op, TokenType::Star);

        auto left_id = dynamic_cast<IdentifierAccess *>(result_bin->left.get());
        ASSERT_NE(left_id, nullptr);
        EXPECT_EQ(left_id->name, "base");
    }

    TEST_F(AstBaseTest, ValidatesSwitchPrattBindingPrecedence) {
        // AST Shape: Assignment -> Binary(+) -> Left: 100, Right: Binary(*) -> Left: Switch, Right: 5

        auto ast = parse_code("let val = 100 + switch (dir) { case UP -> 1 default -> -1 } * 5");

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);

        // The root of the expression should be the '+' operator
        auto root_plus = dynamic_cast<BinaryExpression *>(assignment->value.get());
        ASSERT_NE(root_plus, nullptr) << "Expected root of expression to be addition";
        EXPECT_EQ(root_plus->op, TokenType::Plus);

        // Left side of '+' is 100
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(root_plus->left.get())->value, "100");

        // Right side of '+' is the '*' operator (because * binds tighter than +)
        auto right_star = dynamic_cast<BinaryExpression *>(root_plus->right.get());
        ASSERT_NE(right_star, nullptr) << "Expected right side to be multiplication";
        EXPECT_EQ(right_star->op, TokenType::Star);

        // Left side of '*' is the Switch expression itself!
        auto switch_expr = dynamic_cast<SwitchExpression *>(right_star->left.get());
        ASSERT_NE(switch_expr, nullptr) << "Expected left side of multiplication to be the SwitchExpression";
        EXPECT_EQ(switch_expr->cases[0].first[0], "UP");

        // Right side of '*' is 5
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(right_star->right.get())->value, "5");
    }

    TEST_F(AstBaseTest, ValidatesDeeplyNestedSwitchExpressions) {
        auto ast = parse_code("let a = switch (x) { case A -> switch (y) { case B -> 1 default -> 2 } default -> 3 }");

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);

        // Outer Switch
        auto outer_switch = dynamic_cast<SwitchExpression *>(assignment->value.get());
        ASSERT_NE(outer_switch, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(outer_switch->target.get())->name, "x");
        ASSERT_EQ(outer_switch->cases.size(), 1);
        EXPECT_EQ(outer_switch->cases[0].first[0], "A");

        // Inner Switch (The result of Case A)
        auto inner_switch = dynamic_cast<SwitchExpression *>(outer_switch->cases[0].second.get());
        ASSERT_NE(inner_switch, nullptr) << "Expected case A to resolve to an inner SwitchExpression";
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(inner_switch->target.get())->name, "y");

        ASSERT_EQ(inner_switch->cases.size(), 1);
        EXPECT_EQ(inner_switch->cases[0].first[0], "B");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner_switch->cases[0].second.get())->value, "1");

        ASSERT_NE(inner_switch->default_case, nullptr);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner_switch->default_case.get())->value, "2");

        // Outer Switch Default
        ASSERT_NE(outer_switch->default_case, nullptr);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(outer_switch->default_case.get())->value, "3");
    }

    TEST_F(AstBaseTest, ValidatesSwitchInsideFunctionArguments) {
        auto ast = parse_code("let a = process(p: switch (dir) { case UP, DOWN -> 1 }, s: 42)");

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);

        auto func_call = dynamic_cast<FunctionCall *>(assignment->value.get());
        ASSERT_NE(func_call, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(func_call->target.get())->name, "process");

        // Verify the function has exactly 2 arguments
        ASSERT_EQ(func_call->arguments.size(), 2);

        // Argument 1: The Switch Expression
        auto switch_arg = dynamic_cast<SwitchExpression *>(func_call->arguments[0].second.get());
        ASSERT_NE(switch_arg, nullptr);
        ASSERT_EQ(switch_arg->cases[0].first.size(), 2);
        EXPECT_EQ(switch_arg->cases[0].first[0], "UP");
        EXPECT_EQ(switch_arg->cases[0].first[1], "DOWN");

        // Argument 2: The Number 42
        auto num_arg = dynamic_cast<NumberLiteral *>(func_call->arguments[1].second.get());
        ASSERT_NE(num_arg, nullptr);
        EXPECT_EQ(num_arg->value, "42");
    }

    TEST_F(AstBaseTest, ValidatesDictLiteralAsCaseResult) {
        auto ast = parse_code("let a = switch (res) { case UP -> { score: 100 } default -> { score: 0 } }");

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);

        auto switch_expr = dynamic_cast<SwitchExpression *>(assignment->value.get());
        ASSERT_NE(switch_expr, nullptr);

        // Verify Case Result is a DictLiteral
        auto up_result = dynamic_cast<DictLiteral *>(switch_expr->cases[0].second.get());
        ASSERT_NE(up_result, nullptr);
        ASSERT_EQ(up_result->elements.size(), 1);
        EXPECT_EQ(up_result->elements[0].key, "score");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(up_result->elements[0].value.get())->value, "100");

        // Verify Default is a DictLiteral
        auto default_result = dynamic_cast<DictLiteral *>(switch_expr->default_case.get());
        ASSERT_NE(default_result, nullptr);
        ASSERT_EQ(default_result->elements.size(), 1);
        EXPECT_EQ(default_result->elements[0].key, "score");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(default_result->elements[0].value.get())->value, "0");
    }

    TEST_F(AstBaseTest, ValidatesPostfixChainingOnSwitch) {
        // Evaluates the switch to an object, calls build() on it, and accesses index 0.

        auto ast = parse_code("let a = switch (res) { case UP -> factory default -> fallback }.build()[0]");

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);

        // Root of the value should be the BracketAccess [0]
        auto bracket_acc = dynamic_cast<BracketAccess *>(assignment->value.get());
        ASSERT_NE(bracket_acc, nullptr);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(bracket_acc->index.get())->value, "0");

        // Target of [0] should be the FunctionCall build()
        auto func_call = dynamic_cast<FunctionCall *>(bracket_acc->target.get());
        ASSERT_NE(func_call, nullptr);

        // Target of build() should be the DotAccess .build
        auto dot_acc = dynamic_cast<DotAccess *>(func_call->target.get());
        ASSERT_NE(dot_acc, nullptr);
        EXPECT_EQ(dot_acc->property_name, "build");

        // Target of .build should be the SwitchExpression
        auto switch_expr = dynamic_cast<SwitchExpression *>(dot_acc->target.get());
        ASSERT_NE(switch_expr, nullptr) << "Expected the target of the dot access to be the SwitchExpression";
        EXPECT_EQ(switch_expr->cases[0].first[0], "UP");
    }

    TEST_F(AstBaseTest, ValidatesTensorLiteralAsCaseResult) {
        // Proves that commas inside the vector literal do not interfere with the switch parser's comma logic.

        auto ast = parse_code("let weights = switch (state) { case UP, SUS -> [1.5, 0.5] default -> [1.0, 1.0] }");

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);

        auto switch_expr = dynamic_cast<SwitchExpression *>(assignment->value.get());
        ASSERT_NE(switch_expr, nullptr);

        // 1. Verify the Target
        auto target_id = dynamic_cast<IdentifierAccess *>(switch_expr->target.get());
        ASSERT_NE(target_id, nullptr);
        EXPECT_EQ(target_id->name, "state");

        // 2. Verify Case Result is a TensorLiteral
        ASSERT_EQ(switch_expr->cases.size(), 1);
        EXPECT_EQ(switch_expr->cases[0].first.size(), 2); // UP, SUS

        auto up_result = dynamic_cast<TensorLiteral *>(switch_expr->cases[0].second.get());
        ASSERT_NE(up_result, nullptr) << "Expected case result to be a TensorLiteral";
        ASSERT_EQ(up_result->elements.size(), 2);

        auto up_elem_0 = dynamic_cast<NumberLiteral *>(up_result->elements[0].get());
        ASSERT_NE(up_elem_0, nullptr);
        EXPECT_EQ(up_elem_0->value, "1.5");

        auto up_elem_1 = dynamic_cast<NumberLiteral *>(up_result->elements[1].get());
        ASSERT_NE(up_elem_1, nullptr);
        EXPECT_EQ(up_elem_1->value, "0.5");

        // 3. Verify Default is a TensorLiteral
        ASSERT_NE(switch_expr->default_case, nullptr);

        auto default_result = dynamic_cast<TensorLiteral *>(switch_expr->default_case.get());
        ASSERT_NE(default_result, nullptr) << "Expected default result to be a TensorLiteral";
        ASSERT_EQ(default_result->elements.size(), 2);

        auto def_elem_0 = dynamic_cast<NumberLiteral *>(default_result->elements[0].get());
        ASSERT_NE(def_elem_0, nullptr);
        EXPECT_EQ(def_elem_0->value, "1.0");

        auto def_elem_1 = dynamic_cast<NumberLiteral *>(default_result->elements[1].get());
        ASSERT_NE(def_elem_1, nullptr);
        EXPECT_EQ(def_elem_1->value, "1.0");
    }

    TEST_F(AstBaseTest, ValidatesPercentageInsideSwitchExpression) {
        auto ast = parse_code("let rate = switch (risk_profile) { case HIGH -> 15.5% default -> 4% }");

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);

        auto switch_expr = dynamic_cast<SwitchExpression *>(assignment->value.get());
        ASSERT_NE(switch_expr, nullptr);

        // Verify HIGH branch
        ASSERT_EQ(switch_expr->cases.size(), 1);
        auto high_result = dynamic_cast<PercentageLiteral *>(switch_expr->cases[0].second.get());
        ASSERT_NE(high_result, nullptr) << "Expected switch branch result to be a PercentageLiteral";
        EXPECT_EQ(high_result->value, "15.5%");

        // Verify default branch
        auto default_result = dynamic_cast<PercentageLiteral *>(switch_expr->default_case.get());
        ASSERT_NE(default_result, nullptr) << "Expected switch default result to be a PercentageLiteral";
        EXPECT_EQ(default_result->value, "4%");
    }
}
