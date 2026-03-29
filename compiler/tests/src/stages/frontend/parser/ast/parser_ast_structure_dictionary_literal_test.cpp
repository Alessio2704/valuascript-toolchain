#include "../ast_base_test.h"

namespace valuascript::compiler::test {
    TEST_F(AstBaseTest, ValidatesEmptyDictionary) {
        // Proves the parser correctly identifies opening and closing braces with no contents.

        auto ast = parse_code("let empty = {}");
        auto dict_val = dynamic_cast<DictLiteral *>(get_assigned_value(ast));

        ASSERT_NE(dict_val, nullptr) << "Assigned value must be a DictLiteral";
        EXPECT_EQ(dict_val->elements.size(), 0) << "Empty dictionary must have 0 elements";
    }

    TEST_F(AstBaseTest, ValidatesFlatDictionary) {
        // Proves the parser accurately captures key-value elements with primitive expressions.

        auto ast = parse_code("let model = { cagr: 0.05, yrs: 10 }");
        auto dict_val = dynamic_cast<DictLiteral *>(get_assigned_value(ast));

        ASSERT_NE(dict_val, nullptr);
        ASSERT_EQ(dict_val->elements.size(), 2);

        // First Pair: cagr: 0.05
        EXPECT_EQ(dict_val->elements[0].key, "cagr");
        auto val0 = dynamic_cast<NumberLiteral *>(dict_val->elements[0].value.get());
        ASSERT_NE(val0, nullptr);
        EXPECT_EQ(val0->value, "0.05");

        // Second Pair: yrs: 10
        EXPECT_EQ(dict_val->elements[1].key, "yrs");
        auto val1 = dynamic_cast<NumberLiteral *>(dict_val->elements[1].value.get());
        ASSERT_NE(val1, nullptr);
        EXPECT_EQ(val1->value, "10");
    }

    TEST_F(AstBaseTest, ValidatesNestedDictionaryWithComplexExpressions) {
        // Proves that dictionaries safely house nested dictionaries and math expressions
        // without corrupting the parser's internal loop state.

        auto ast = parse_code("let nested = { base: { rate: 0.05 }, stress: wacc * 1.2 }");
        auto dict_val = dynamic_cast<DictLiteral *>(get_assigned_value(ast));

        ASSERT_NE(dict_val, nullptr);
        ASSERT_EQ(dict_val->elements.size(), 2);

        // ==========================================
        // PAIR 0: base: { rate: 0.05 }
        // ==========================================
        EXPECT_EQ(dict_val->elements[0].key, "base");

        auto inner_dict = dynamic_cast<DictLiteral *>(dict_val->elements[0].value.get());
        ASSERT_NE(inner_dict, nullptr) << "Value for 'base' must be a nested DictLiteral";
        ASSERT_EQ(inner_dict->elements.size(), 1);

        EXPECT_EQ(inner_dict->elements[0].key, "rate");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner_dict->elements[0].value.get())->value, "0.05");

        // ==========================================
        // PAIR 1: stress: wacc * 1.2
        // ==========================================
        EXPECT_EQ(dict_val->elements[1].key, "stress");

        auto math_op = dynamic_cast<BinaryExpression *>(dict_val->elements[1].value.get());
        ASSERT_NE(math_op, nullptr) << "Value for 'stress' must be a BinaryExpression";
        EXPECT_EQ(math_op->op, TokenType::Star);

        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(math_op->left.get())->name, "wacc");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(math_op->right.get())->value, "1.2");
    }

    TEST_F(AstBaseTest, ValidatesOmnibusDictionaryWithAllExpressionTypes) {
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
        auto dict_val = dynamic_cast<DictLiteral *>(get_assigned_value(ast));

        ASSERT_NE(dict_val, nullptr) << "Assigned value must be a DictLiteral";
        ASSERT_EQ(dict_val->elements.size(), 8) << "Omnibus dictionary must hold exactly 8 elements";

        // ==========================================
        // PAIR 0: Unary Expression (scalar: -100)
        // ==========================================
        EXPECT_EQ(dict_val->elements[0].key, "scalar");
        auto pair0_unary = dynamic_cast<UnaryExpression *>(dict_val->elements[0].value.get());
        ASSERT_NE(pair0_unary, nullptr);
        EXPECT_EQ(pair0_unary->op, TokenType::Minus);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(pair0_unary->right.get())->value, "100");

        // ==========================================
        // PAIR 1: Overridden Binary Math (equation: (base + 0.05) * multiplier)
        // ==========================================
        EXPECT_EQ(dict_val->elements[1].key, "equation");
        auto pair1_mult = dynamic_cast<BinaryExpression *>(dict_val->elements[1].value.get());
        ASSERT_NE(pair1_mult, nullptr);
        EXPECT_EQ(pair1_mult->op, TokenType::Star);

        auto pair1_add = dynamic_cast<BinaryExpression *>(unwrap(pair1_mult->left.get()));
        ASSERT_NE(pair1_add, nullptr);
        EXPECT_EQ(pair1_add->op, TokenType::Plus);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(pair1_mult->right.get())->name, "multiplier");

        // ==========================================
        // PAIR 2: Logical Unary (logic: not is_valid)
        // ==========================================
        EXPECT_EQ(dict_val->elements[2].key, "logic");
        auto pair2_not = dynamic_cast<UnaryExpression *>(dict_val->elements[2].value.get());
        ASSERT_NE(pair2_not, nullptr);
        // Adjust token type based on your enum for the 'not' keyword
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(pair2_not->right.get())->name, "is_valid");

        // ==========================================
        // PAIR 3: Tuple Literal (group: (1, a * b))
        // ==========================================
        EXPECT_EQ(dict_val->elements[3].key, "group");
        auto pair3_tuple = dynamic_cast<TupleLiteral *>(dict_val->elements[3].value.get());
        ASSERT_NE(pair3_tuple, nullptr);
        ASSERT_EQ(pair3_tuple->elements.size(), 2);
        EXPECT_EQ(dynamic_cast<BinaryExpression*>(pair3_tuple->elements[1].get())->op, TokenType::Star);

        // ==========================================
        // PAIR 4: Vector Literal (arr: [10, 20])
        // ==========================================
        EXPECT_EQ(dict_val->elements[4].key, "arr");
        auto pair4_vec = dynamic_cast<TensorLiteral *>(dict_val->elements[4].value.get());
        ASSERT_NE(pair4_vec, nullptr);
        ASSERT_EQ(pair4_vec->elements.size(), 2);

        // ==========================================
        // PAIR 5: Tensor Slicing (subset: history[0 : 10])
        // ==========================================
        EXPECT_EQ(dict_val->elements[5].key, "subset");
        auto pair5_slice = dynamic_cast<BracketAccess *>(dict_val->elements[5].value.get());
        ASSERT_NE(pair5_slice, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(pair5_slice->target.get())->name, "history");
        EXPECT_EQ(dynamic_cast<BinaryExpression*>(pair5_slice->index.get())->op, TokenType::Colon);

        // ==========================================
        // PAIR 6: Function Call (invoke: calc_risk(rate: 0.08))
        // ==========================================
        EXPECT_EQ(dict_val->elements[6].key, "invoke");
        auto pair6_call = dynamic_cast<FunctionCall *>(dict_val->elements[6].value.get());
        ASSERT_NE(pair6_call, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(pair6_call->target.get())->name, "calc_risk");
        ASSERT_EQ(pair6_call->arguments.size(), 1);

        // ==========================================
        // PAIR 7: Nested Dict with Chained Unary (nested: { inner: !!flag })
        // ==========================================
        EXPECT_EQ(dict_val->elements[7].key, "nested");
        auto pair7_dict = dynamic_cast<DictLiteral *>(dict_val->elements[7].value.get());
        ASSERT_NE(pair7_dict, nullptr);
        ASSERT_EQ(pair7_dict->elements.size(), 1);

        EXPECT_EQ(pair7_dict->elements[0].key, "inner");
        auto inner_unary_outer = dynamic_cast<UnaryExpression *>(pair7_dict->elements[0].value.get());
        ASSERT_NE(inner_unary_outer, nullptr);
        EXPECT_EQ(inner_unary_outer->op, TokenType::Not);

        auto inner_unary_inner = dynamic_cast<UnaryExpression *>(inner_unary_outer->right.get());
        ASSERT_NE(inner_unary_inner, nullptr);
        EXPECT_EQ(inner_unary_inner->op, TokenType::Not);
    }
}
