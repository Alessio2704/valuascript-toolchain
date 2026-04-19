#include "frontend/parser/helpers/ast_base_test.h"

namespace valuascript::compiler::test
{
    TEST_F(AstBaseTest, ValidatesEmptyDictionary)
    {
        // Proves the parser correctly identifies opening and closing braces with no contents.

        auto ast = parse_code("let empty = {}");
        auto dict_val = dynamic_cast<DictLiteral*>(get_assigned_value(ast));

        ASSERT_NE(dict_val, nullptr) << "Assigned value must be a DictLiteral";
        EXPECT_EQ(dict_val->elements.size(), 0) << "Empty dictionary must have 0 elements";
    }

    TEST_F(AstBaseTest, ValidatesFlatDictionary)
    {
        // Proves the parser accurately captures key-value elements with primitive expressions.

        auto ast = parse_code("let model = { cagr: 0.05, yrs: 10 }");
        auto dict_val = dynamic_cast<DictLiteral*>(get_assigned_value(ast));

        ASSERT_NE(dict_val, nullptr);
        ASSERT_EQ(dict_val->elements.size(), 2);

        // First Pair: cagr: 0.05
        EXPECT_EQ(dict_val->elements[0].key, "cagr");
        auto val0 = dynamic_cast<NumberLiteral*>(dict_val->elements[0].value.get());
        ASSERT_NE(val0, nullptr);
        EXPECT_EQ(val0->value, "0.05");

        // Second Pair: yrs: 10
        EXPECT_EQ(dict_val->elements[1].key, "yrs");
        auto val1 = dynamic_cast<NumberLiteral*>(dict_val->elements[1].value.get());
        ASSERT_NE(val1, nullptr);
        EXPECT_EQ(val1->value, "10");
    }

    TEST_F(AstBaseTest, ValidatesNestedDictionaryWithComplexExpressions)
    {
        // Proves that dictionaries safely house nested dictionaries and math expressions
        // without corrupting the parser's internal loop state.

        auto ast = parse_code("let nested = { base: { rate: 0.05 }, stress: wacc * 1.2 }");
        auto dict_val = dynamic_cast<DictLiteral*>(get_assigned_value(ast));

        ASSERT_NE(dict_val, nullptr);
        ASSERT_EQ(dict_val->elements.size(), 2);

        // ==========================================
        // PAIR 0: base: { rate: 0.05 }
        // ==========================================
        EXPECT_EQ(dict_val->elements[0].key, "base");

        auto inner_dict = dynamic_cast<DictLiteral*>(dict_val->elements[0].value.get());
        ASSERT_NE(inner_dict, nullptr) << "Value for 'base' must be a nested DictLiteral";
        ASSERT_EQ(inner_dict->elements.size(), 1);

        EXPECT_EQ(inner_dict->elements[0].key, "rate");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner_dict->elements[0].value.get())->value, "0.05");

        // ==========================================
        // PAIR 1: stress: wacc * 1.2
        // ==========================================
        EXPECT_EQ(dict_val->elements[1].key, "stress");

        auto math_op = dynamic_cast<BinaryExpression*>(dict_val->elements[1].value.get());
        ASSERT_NE(math_op, nullptr) << "Value for 'stress' must be a BinaryExpression";
        EXPECT_EQ(math_op->op, TokenType::Star);

        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(math_op->left.get())->name, "wacc");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(math_op->right.get())->value, "1.2");
    }

    TEST_F(AstBaseTest, ValidatesOmnibusDictionaryWithAllExpressionTypes)
    {
        // Proves the DictLiteral seamlessly delegates parsing to every corner of the expression grammar.

        std::string code =
            "let omnibus = {\n"
            "    scalar: -100,\n"
            "    equation: (base + 0.05) * multiplier,\n"
            "    logic: not is_valid,\n"
            "    group: (1, a * b),\n"
            "    arr: [10, 20],\n"
            "    subset: history[0 : 10],\n"
            "    invoke: calc_risk(rate: 0.08),\n"
            "    nested: { inner: not not flag }\n"
            "}";

        auto ast = parse_code(code);
        auto dict_val = dynamic_cast<DictLiteral*>(get_assigned_value(ast));

        ASSERT_NE(dict_val, nullptr) << "Assigned value must be a DictLiteral";
        ASSERT_EQ(dict_val->elements.size(), 8) << "Omnibus dictionary must hold exactly 8 elements";

        // ==========================================
        // PAIR 0: Unary Expression (scalar: -100)
        // ==========================================
        EXPECT_EQ(dict_val->elements[0].key, "scalar");
        auto pair0_unary = dynamic_cast<UnaryExpression*>(dict_val->elements[0].value.get());
        ASSERT_NE(pair0_unary, nullptr);
        EXPECT_EQ(pair0_unary->op, TokenType::Minus);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(pair0_unary->right.get())->value, "100");

        // ==========================================
        // PAIR 1: Overridden Binary Math (equation: (base + 0.05) * multiplier)
        // ==========================================
        EXPECT_EQ(dict_val->elements[1].key, "equation");
        auto pair1_mult = dynamic_cast<BinaryExpression*>(dict_val->elements[1].value.get());
        ASSERT_NE(pair1_mult, nullptr);
        EXPECT_EQ(pair1_mult->op, TokenType::Star);

        auto pair1_add = dynamic_cast<BinaryExpression*>(unwrap_grouping(pair1_mult->left.get()));
        ASSERT_NE(pair1_add, nullptr);
        EXPECT_EQ(pair1_add->op, TokenType::Plus);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(pair1_mult->right.get())->name, "multiplier");

        // ==========================================
        // PAIR 2: Logical Unary (logic: not is_valid)
        // ==========================================
        EXPECT_EQ(dict_val->elements[2].key, "logic");
        auto pair2_not = dynamic_cast<UnaryExpression*>(dict_val->elements[2].value.get());
        ASSERT_NE(pair2_not, nullptr);
        // Adjust token type based on your enum for the 'not' keyword
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(pair2_not->right.get())->name, "is_valid");

        // ==========================================
        // PAIR 3: Tuple Literal (group: (1, a * b))
        // ==========================================
        EXPECT_EQ(dict_val->elements[3].key, "group");
        auto pair3_tuple = dynamic_cast<TupleLiteral*>(dict_val->elements[3].value.get());
        ASSERT_NE(pair3_tuple, nullptr);
        ASSERT_EQ(pair3_tuple->elements.size(), 2);
        EXPECT_EQ(dynamic_cast<BinaryExpression*>(pair3_tuple->elements[1].get())->op, TokenType::Star);

        // ==========================================
        // PAIR 4: Vector Literal (arr: [10, 20])
        // ==========================================
        EXPECT_EQ(dict_val->elements[4].key, "arr");
        auto pair4_vec = dynamic_cast<TensorLiteral*>(dict_val->elements[4].value.get());
        ASSERT_NE(pair4_vec, nullptr);
        ASSERT_EQ(pair4_vec->elements.size(), 2);

        // ==========================================
        // PAIR 5: Tensor Slicing (subset: history[0 : 10])
        // ==========================================
        EXPECT_EQ(dict_val->elements[5].key, "subset");
        auto pair5_slice = dynamic_cast<BracketAccess*>(dict_val->elements[5].value.get());
        ASSERT_NE(pair5_slice, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(pair5_slice->target.get())->name, "history");
        EXPECT_EQ(dynamic_cast<BinaryExpression*>(pair5_slice->index.get())->op, TokenType::Colon);

        // ==========================================
        // PAIR 6: Function Call (invoke: calc_risk(rate: 0.08))
        // ==========================================
        EXPECT_EQ(dict_val->elements[6].key, "invoke");
        auto pair6_call = dynamic_cast<FunctionCall*>(dict_val->elements[6].value.get());
        ASSERT_NE(pair6_call, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(pair6_call->target.get())->name, "calc_risk");
        ASSERT_EQ(pair6_call->arguments.size(), 1);

        // ==========================================
        // PAIR 7: Nested Dict with Chained Unary (nested: { inner: !!flag })
        // ==========================================
        EXPECT_EQ(dict_val->elements[7].key, "nested");
        auto pair7_dict = dynamic_cast<DictLiteral*>(dict_val->elements[7].value.get());
        ASSERT_NE(pair7_dict, nullptr);
        ASSERT_EQ(pair7_dict->elements.size(), 1);

        EXPECT_EQ(pair7_dict->elements[0].key, "inner");
        auto inner_unary_outer = dynamic_cast<UnaryExpression*>(pair7_dict->elements[0].value.get());
        ASSERT_NE(inner_unary_outer, nullptr);
        EXPECT_EQ(inner_unary_outer->op, TokenType::Not);

        auto inner_unary_inner = dynamic_cast<UnaryExpression*>(inner_unary_outer->right.get());
        ASSERT_NE(inner_unary_inner, nullptr);
        EXPECT_EQ(inner_unary_inner->op, TokenType::Not);
    }

    TEST_F(AstBaseTest, ValidatesSelfPropertyAccess)
    {
        // Proves the parser correctly forms a DotAccess node with a SelfExpression target.

        auto ast = parse_code("let obj = { a: 1, b: self.a }");
        auto dict_val = dynamic_cast<DictLiteral*>(get_assigned_value(ast));

        ASSERT_NE(dict_val, nullptr);
        ASSERT_EQ(dict_val->elements.size(), 2);

        EXPECT_EQ(dict_val->elements[1].key, "b");
        auto b_val = dynamic_cast<DotAccess*>(dict_val->elements[1].value.get());
        ASSERT_NE(b_val, nullptr) << "Value for 'b' must be a DotAccess expression";

        EXPECT_EQ(b_val->property_name, "a");
        auto self_expr = dynamic_cast<SelfExpression*>(b_val->target.get());
        ASSERT_NE(self_expr, nullptr) << "Target of the DotAccess must be a SelfExpression";
    }

    TEST_F(AstBaseTest, ValidatesDeeplyChainedSelfAccess)
    {
        // Proves the parser correctly handles highly nested property access chaining on 'self' (e.g., self.a.b.c).

        auto ast = parse_code("let obj = { nested: { deep: { val: 42 } }, ref: self.nested.deep.val }");
        auto dict_val = dynamic_cast<DictLiteral*>(get_assigned_value(ast));

        ASSERT_NE(dict_val, nullptr);
        ASSERT_EQ(dict_val->elements.size(), 2);

        // Analyze: self.nested.deep.val
        auto val_access = dynamic_cast<DotAccess*>(dict_val->elements[1].value.get());
        ASSERT_NE(val_access, nullptr);
        EXPECT_EQ(val_access->property_name, "val");

        // -> target: self.nested.deep
        auto deep_access = dynamic_cast<DotAccess*>(val_access->target.get());
        ASSERT_NE(deep_access, nullptr);
        EXPECT_EQ(deep_access->property_name, "deep");

        // -> target: self.nested
        auto nested_access = dynamic_cast<DotAccess*>(deep_access->target.get());
        ASSERT_NE(nested_access, nullptr);
        EXPECT_EQ(nested_access->property_name, "nested");

        // -> target: self
        auto self_expr = dynamic_cast<SelfExpression*>(nested_access->target.get());
        ASSERT_NE(self_expr, nullptr);
    }

    TEST_F(AstBaseTest, ValidatesSelfUsedWithBracketAndMethodCall)
    {
        // Proves that 'self' supports matrix/dictionary bracket access, and can be invoked as a method target.

        auto ast = parse_code(
            "let obj = { matrix: [[1]], data: self[\"matrix\"][0], fetch: self.calc(arg: self.data) }");
        auto dict_val = dynamic_cast<DictLiteral*>(get_assigned_value(ast));

        ASSERT_NE(dict_val, nullptr);
        ASSERT_EQ(dict_val->elements.size(), 3);

        // ==========================================
        // PAIR 1: data: self["matrix"][0]
        // ==========================================
        auto outer_bracket = dynamic_cast<BracketAccess*>(dict_val->elements[1].value.get());
        ASSERT_NE(outer_bracket, nullptr);
        EXPECT_EQ(dynamic_cast<NumberLiteral *>(outer_bracket->index.get())->value, "0");

        auto inner_bracket = dynamic_cast<BracketAccess*>(outer_bracket->target.get());
        ASSERT_NE(inner_bracket, nullptr);
        EXPECT_EQ(dynamic_cast<StringLiteral *>(inner_bracket->index.get())->value, "\"matrix\"");

        ASSERT_NE(dynamic_cast<SelfExpression *>(inner_bracket->target.get()), nullptr);

        // ==========================================
        // PAIR 2: fetch: self.calc(arg: self.data)
        // ==========================================
        auto func_call = dynamic_cast<FunctionCall*>(dict_val->elements[2].value.get());
        ASSERT_NE(func_call, nullptr);

        auto target_method = dynamic_cast<DotAccess*>(func_call->target.get());
        ASSERT_NE(target_method, nullptr);
        EXPECT_EQ(target_method->property_name, "calc");
        ASSERT_NE(dynamic_cast<SelfExpression *>(target_method->target.get()), nullptr);

        ASSERT_EQ(func_call->arguments.size(), 1);
        EXPECT_EQ(func_call->arguments[0].first, "arg");

        auto arg_value = dynamic_cast<DotAccess*>(func_call->arguments[0].second.get());
        ASSERT_NE(arg_value, nullptr);
        EXPECT_EQ(arg_value->property_name, "data");
        ASSERT_NE(dynamic_cast<SelfExpression *>(arg_value->target.get()), nullptr);
    }

    TEST_F(AstBaseTest, ValidatesOmnibusSelfExpressions)
    {
        // Proves 'self' behaves perfectly as a primary expression embedded anywhere inside
        // logical, mathematical, tuple, grouping, and conditional constructs.

        std::string code =
            "let omnibus = {\n"
            "    a: 10,\n"
            "    b: 20,\n"
            "    math: (self.a + self.b) / self[\"a\"],\n"
            "    logic: not self.is_ready,\n"
            "    cond: if self.flag then self.x else self.y,\n"
            "    nested_tuple: (self.a, { inner: self.b })\n"
            "}";

        auto ast = parse_code(code);
        auto dict_val = dynamic_cast<DictLiteral*>(get_assigned_value(ast));

        ASSERT_NE(dict_val, nullptr);
        ASSERT_EQ(dict_val->elements.size(), 6);

        // ==========================================
        // math: (self.a + self.b) / self["a"]
        // ==========================================
        auto math_div = dynamic_cast<BinaryExpression*>(dict_val->elements[2].value.get());
        ASSERT_NE(math_div, nullptr);
        EXPECT_EQ(math_div->op, TokenType::Slash);

        auto math_add = dynamic_cast<BinaryExpression*>(unwrap_grouping(math_div->left.get()));
        ASSERT_NE(math_add, nullptr);
        ASSERT_NE(dynamic_cast<SelfExpression *>(dynamic_cast<DotAccess *>(math_add->left.get())->target.get()),
                  nullptr);
        ASSERT_NE(dynamic_cast<SelfExpression *>(dynamic_cast<DotAccess *>(math_add->right.get())->target.get()),
                  nullptr);

        auto math_right_bracket = dynamic_cast<BracketAccess*>(math_div->right.get());
        ASSERT_NE(math_right_bracket, nullptr);
        ASSERT_NE(dynamic_cast<SelfExpression *>(math_right_bracket->target.get()), nullptr);

        // ==========================================
        // logic: not self.is_ready
        // ==========================================
        auto logic_not = dynamic_cast<UnaryExpression*>(dict_val->elements[3].value.get());
        ASSERT_NE(logic_not, nullptr);
        auto logic_dot = dynamic_cast<DotAccess*>(logic_not->right.get());
        ASSERT_NE(logic_dot, nullptr);
        ASSERT_NE(dynamic_cast<SelfExpression *>(logic_dot->target.get()), nullptr);

        // ==========================================
        // cond: if self.flag then self.x else self.y
        // ==========================================
        auto cond_expr = dynamic_cast<ConditionalExpression*>(dict_val->elements[4].value.get());
        ASSERT_NE(cond_expr, nullptr);

        ASSERT_NE(dynamic_cast<SelfExpression *>(dynamic_cast<DotAccess *>(cond_expr->condition.get())->target.get()),
                  nullptr);
        ASSERT_NE(dynamic_cast<SelfExpression *>(dynamic_cast<DotAccess *>(cond_expr->then_branch.get())->target.get()),
                  nullptr);
        ASSERT_NE(dynamic_cast<SelfExpression *>(dynamic_cast<DotAccess *>(cond_expr->else_branch.get())->target.get()),
                  nullptr);

        // ==========================================
        // nested_tuple: (self.a, { inner: self.b })
        // ==========================================
        auto tuple_expr = dynamic_cast<TupleLiteral*>(dict_val->elements[5].value.get());
        ASSERT_NE(tuple_expr, nullptr);
        ASSERT_EQ(tuple_expr->elements.size(), 2);

        ASSERT_NE(
            dynamic_cast<SelfExpression *>(dynamic_cast<DotAccess *>(tuple_expr->elements[0].get())->target.get()),
            nullptr);

        auto inner_dict = dynamic_cast<DictLiteral*>(tuple_expr->elements[1].get());
        ASSERT_NE(inner_dict, nullptr);
        ASSERT_NE(
            dynamic_cast<SelfExpression *>(dynamic_cast<DotAccess *>(inner_dict->elements[0].value.get())->target.get()
            ), nullptr);
    }

    TEST_F(AstBaseTest, ValidatesSelfInSwitchExpressions)
    {
        // Proves 'self' can safely be used as both the target of a switch and inside the case results.

        std::string code =
            "let obj = {\n"
            "    val: switch (self.state) {\n"
            "        case Active -> self.on_val\n"
            "        default -> self.off_val\n"
            "    }\n"
            "}";

        auto ast = parse_code(code);
        auto dict_val = dynamic_cast<DictLiteral*>(get_assigned_value(ast));

        ASSERT_NE(dict_val, nullptr);
        ASSERT_EQ(dict_val->elements.size(), 1);

        auto switch_expr = dynamic_cast<SwitchExpression*>(dict_val->elements[0].value.get());
        ASSERT_NE(switch_expr, nullptr);

        // Target: self.state
        auto switch_target = dynamic_cast<DotAccess*>(switch_expr->target.get());
        ASSERT_NE(switch_target, nullptr);
        EXPECT_EQ(switch_target->property_name, "state");
        ASSERT_NE(dynamic_cast<SelfExpression *>(switch_target->target.get()), nullptr);

        // Case Result: self.on_val
        ASSERT_EQ(switch_expr->cases.size(), 1);
        auto case_result = dynamic_cast<DotAccess*>(switch_expr->cases[0].second.get());
        ASSERT_NE(case_result, nullptr);
        EXPECT_EQ(case_result->property_name, "on_val");
        ASSERT_NE(dynamic_cast<SelfExpression *>(case_result->target.get()), nullptr);

        // Default Result: self.off_val
        auto default_result = dynamic_cast<DotAccess*>(switch_expr->default_case.get());
        ASSERT_NE(default_result, nullptr);
        EXPECT_EQ(default_result->property_name, "off_val");
        ASSERT_NE(dynamic_cast<SelfExpression *>(default_result->target.get()), nullptr);
    }

    TEST_F(AstBaseTest, ValidatesDictLiteralWithMixedModifiedAndUnmodifiedKeys)
    {
        auto ast = parse_code(
            "let data = { "
            "normal_key: 1, "
            "@special flagged_key: 2, "
            "another_normal: 3 "
            "}");

        auto assign_node = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
        auto dict_lit = dynamic_cast<DictLiteral*>(assign_node->value.get());
        ASSERT_NE(dict_lit, nullptr);
        ASSERT_EQ(dict_lit->elements.size(), 3);

        EXPECT_TRUE(dict_lit->elements[0].modifiers.empty());

        ASSERT_EQ(dict_lit->elements[1].modifiers.size(), 1);
        EXPECT_EQ(dict_lit->elements[1].modifiers[0].name, "special");

        EXPECT_TRUE(dict_lit->elements[2].modifiers.empty());
    }

    TEST_F(AstBaseTest, ValidatesNestedDictsWithModifiedKeys)
    {
        auto ast = parse_code(
            "let payload = { "
            "  @root config: { "
            "    @nested retries: 3 "
            "  } "
            "}");

        auto assign_node = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
        auto outer_dict = dynamic_cast<DictLiteral*>(assign_node->value.get());
        ASSERT_NE(outer_dict, nullptr);
        ASSERT_EQ(outer_dict->elements.size(), 1);

        // Check Outer Dict Key
        const auto& config_item = outer_dict->elements[0];
        EXPECT_EQ(config_item.key, "config");
        ASSERT_EQ(config_item.modifiers.size(), 1);
        EXPECT_EQ(config_item.modifiers[0].name, "root");

        // Check Inner Dict
        auto inner_dict = dynamic_cast<DictLiteral*>(config_item.value.get());
        ASSERT_NE(inner_dict, nullptr);
        ASSERT_EQ(inner_dict->elements.size(), 1);

        // Check Inner Dict Key
        const auto& retries_item = inner_dict->elements[0];
        EXPECT_EQ(retries_item.key, "retries");
        ASSERT_EQ(retries_item.modifiers.size(), 1);
        EXPECT_EQ(retries_item.modifiers[0].name, "nested");
        EXPECT_EQ(dynamic_cast<NumberLiteral *>(retries_item.value.get())->value, "3");
    }

    TEST_F(AstBaseTest, ValidatesModifierWithEmptyParensAndTrailingCommaOnDictKey)
    {
        auto ast = parse_code(
            "let meta = { "
            "@internal() data: 1, "
            "}");

        auto assign_node = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
        auto dict_lit = dynamic_cast<DictLiteral*>(assign_node->value.get());
        ASSERT_NE(dict_lit, nullptr);
        ASSERT_EQ(dict_lit->elements.size(), 1);

        const auto& item = dict_lit->elements[0];
        ASSERT_EQ(item.modifiers.size(), 1);
        EXPECT_EQ(item.modifiers[0].name, "internal");
        EXPECT_TRUE(item.modifiers[0].arguments.empty());
    }
}
