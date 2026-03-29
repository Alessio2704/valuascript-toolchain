#include <gtest/gtest.h>
#include "../ast_base_test.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    class AstSpanTest : public AstBaseTest {
    protected:
        static void assert_span(const SourceSpan &span, size_t line_start, size_t col_start, size_t line_end,
                                size_t col_end) {
            EXPECT_EQ(span.line_start, line_start) << "Mismatch in line_start";
            EXPECT_EQ(span.column_start, col_start) << "Mismatch in column_start";
            EXPECT_EQ(span.line_end, line_end) << "Mismatch in line_end";
            EXPECT_EQ(span.column_end, col_end) << "Mismatch in column_end";
            EXPECT_EQ(span.file_path, "test.vs") << "Mismatch in file_path";
        }
    };

    TEST_F(AstSpanTest, ValidatesSimpleAssignmentAndLiteralSpans) {
        auto ast = parse_code("let x = 42");

        ASSERT_EQ(ast->execution_steps.size(), 1);
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assign_node, nullptr);

        // Span for the entire assignment: 'let' starts at 1, '42' ends at 11
        assert_span(assign_node->span, 1, 1, 1, 11);

        // Span for the value (NumberLiteral): '42' starts at 9, length 2, ends at 11
        auto num_lit = dynamic_cast<NumberLiteral *>(assign_node->value.get());
        ASSERT_NE(num_lit, nullptr);
        assert_span(num_lit->span, 1, 9, 1, 11);
    }

    TEST_F(AstSpanTest, ValidatesBinaryExpressionSubSpans) {
        auto ast = parse_code("let y = 10 + 20 * 3");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assign_node, nullptr);

        auto root_plus = dynamic_cast<BinaryExpression *>(assign_node->value.get());
        ASSERT_NE(root_plus, nullptr);

        // Entire expression '10 + 20 * 3'
        assert_span(root_plus->span, 1, 9, 1, 20);

        auto left_10 = dynamic_cast<NumberLiteral *>(root_plus->left.get());
        ASSERT_NE(left_10, nullptr);
        assert_span(left_10->span, 1, 9, 1, 11);

        auto right_mult = dynamic_cast<BinaryExpression *>(root_plus->right.get());
        ASSERT_NE(right_mult, nullptr);
        // '20 * 3' spans from col 14 to col 20
        assert_span(right_mult->span, 1, 14, 1, 20);
    }

    TEST_F(AstSpanTest, ValidatesMultilineFunctionDefinitionSpans) {
        std::string code =
                "func add(a: int) -> int {\n"
                "    return a + 1\n"
                "}";

        auto ast = parse_code(code);
        ASSERT_EQ(ast->function_definitions.size(), 1);

        auto func_def = ast->function_definitions[0].get();

        // Function definition spans from 'func' on line 1 to '}' on line 3
        assert_span(func_def->span, 1, 1, 3, 2); // '}' is col 1, length 1, ends at 2

        ASSERT_EQ(func_def->body.size(), 1);
        auto ret_stmt = dynamic_cast<ReturnStatement *>(func_def->body[0].get());
        ASSERT_NE(ret_stmt, nullptr);

        // 'return a + 1' on line 2
        // 'return' starts at col 5. '1' ends at col 17.
        assert_span(ret_stmt->span, 2, 5, 2, 17);
    }

    TEST_F(AstSpanTest, ValidatesNestedFunctionCallSpans) {
        auto ast = parse_code("let z = outer(p: inner(p: 42), s: \"test\")");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assign_node, nullptr);

        auto outer_call = dynamic_cast<FunctionCall *>(assign_node->value.get());
        ASSERT_NE(outer_call, nullptr);

        // Entire call 'outer(inner(p: 42), s: "test")'
        assert_span(outer_call->span, 1, 9, 1, 42);

        // First argument 'inner(p: 42)'
        auto inner_call = dynamic_cast<FunctionCall *>(outer_call->arguments[0].second.get());
        ASSERT_NE(inner_call, nullptr);
        assert_span(inner_call->span, 1, 18, 1, 30);

        // Second argument '"test"'
        auto str_lit = dynamic_cast<StringLiteral *>(outer_call->arguments[1].second.get());
        ASSERT_NE(str_lit, nullptr);
        assert_span(str_lit->span, 1, 35, 1, 41); // starts at 35, length 6 (including quotes) -> 41
    }

    TEST_F(AstSpanTest, ValidatesBracketAccessAndTensorSpans) {
        auto ast = parse_code("let arr = [1, 2][0]");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assign_node, nullptr);

        auto bracket_access = dynamic_cast<BracketAccess *>(assign_node->value.get());
        ASSERT_NE(bracket_access, nullptr);

        // Entire '[1, 2][0]' expression
        assert_span(bracket_access->span, 1, 11, 1, 20);

        auto tensor_lit = dynamic_cast<TensorLiteral *>(bracket_access->target.get());
        ASSERT_NE(tensor_lit, nullptr);

        // The base tensor '[1, 2]'
        assert_span(tensor_lit->span, 1, 11, 1, 17);
    }

    TEST_F(AstSpanTest, ValidatesStructDefinitionSpans) {
        // struct Point { x: float, y: float }
        auto ast = parse_code("struct Point { x: float, y: float }");

        ASSERT_EQ(ast->struct_definitions.size(), 1);
        auto struct_def = ast->struct_definitions[0].get();

        // Entire struct spans from 'struct' (col 1) to '}' (col 35) -> ends at 36
        assert_span(struct_def->span, 1, 1, 1, 36);

        // Check first field type annotation 'float' starts at 19, ends at 24
        auto field1_type = struct_def->fields[0].second.get();
        ASSERT_NE(field1_type, nullptr);
        assert_span(field1_type->span, 1, 19, 1, 24);
    }

    TEST_F(AstSpanTest, ValidatesTensorLiteralSpans) {
        auto ast = parse_code("let t = [1, 2, 3]");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());

        auto tensor = dynamic_cast<TensorLiteral *>(assign_node->value.get());
        ASSERT_NE(tensor, nullptr);

        // '[' is at 9, ']' is at 17 -> ends at 18
        assert_span(tensor->span, 1, 9, 1, 18);

        auto first_elem = dynamic_cast<NumberLiteral *>(tensor->elements[0].get());
        ASSERT_NE(first_elem, nullptr);
        assert_span(first_elem->span, 1, 10, 1, 11);
    }

    TEST_F(AstSpanTest, ValidatesDotAndBracketAccessSpans) {
        auto ast = parse_code("let v = obj.prop[0]");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());

        auto bracket_access = dynamic_cast<BracketAccess *>(assign_node->value.get());
        ASSERT_NE(bracket_access, nullptr);

        // Entire expression 'obj.prop[0]' spans 9 to 19 -> ends at 20
        assert_span(bracket_access->span, 1, 9, 1, 20);

        auto dot_access = dynamic_cast<DotAccess *>(bracket_access->target.get());
        ASSERT_NE(dot_access, nullptr);

        // 'obj.prop' spans 9 to 17 -> ends at 17
        assert_span(dot_access->span, 1, 9, 1, 17);

        // 'obj' spans 9 to 12
        auto identifier = dynamic_cast<IdentifierAccess *>(dot_access->target.get());
        ASSERT_NE(identifier, nullptr);
        assert_span(identifier->span, 1, 9, 1, 12);
    }

    TEST_F(AstSpanTest, ValidatesUnaryAndBinaryOpSpans) {
        auto ast = parse_code("let r = -x + not y");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());

        auto binary_op = dynamic_cast<BinaryExpression *>(assign_node->value.get());
        ASSERT_NE(binary_op, nullptr);

        // Entire '-x + not y' spans 9 to 19 (ends at 19)
        assert_span(binary_op->span, 1, 9, 1, 19);

        auto left_unary = dynamic_cast<UnaryExpression *>(binary_op->left.get());
        ASSERT_NE(left_unary, nullptr);

        // '-x' spans 9 to 11
        assert_span(left_unary->span, 1, 9, 1, 11);

        auto right_unary = dynamic_cast<UnaryExpression *>(binary_op->right.get());
        ASSERT_NE(right_unary, nullptr);

        // 'not y' spans 14 to 19
        assert_span(right_unary->span, 1, 14, 1, 19);
    }

    TEST_F(AstSpanTest, ValidatesTupleAndTupleAnnotationSpans) {
        auto ast = parse_code("let pair: (int, int) = (1, 2)");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());

        auto tuple_ann = dynamic_cast<TupleTypeAnnotation *>(assign_node->targets[0].second.get());
        ASSERT_NE(tuple_ann, nullptr);

        // '(int, int)' starts at 11, ends at 20 -> ends at 21
        assert_span(tuple_ann->span, 1, 11, 1, 21);

        auto tuple_lit = dynamic_cast<TupleLiteral *>(assign_node->value.get());
        ASSERT_NE(tuple_lit, nullptr);

        // '(1, 2)' starts at 24, ends at 29 -> ends at 30
        assert_span(tuple_lit->span, 1, 24, 1, 30);
    }

    TEST_F(AstSpanTest, ValidatesSwitchExpressionSpans) {
        auto ast = parse_code("let s = switch(x) { case a -> 1 default -> 2 }");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());

        auto switch_expr = dynamic_cast<SwitchExpression *>(assign_node->value.get());
        ASSERT_NE(switch_expr, nullptr);

        // 'switch' starts at 9, '}' is at 46 -> ends at 47
        assert_span(switch_expr->span, 1, 9, 1, 47);

        // Target 'x' is at 16 -> ends at 17
        assert_span(switch_expr->target->span, 1, 16, 1, 17);

        // Case 0 value '1' is at 31 -> ends at 32
        assert_span(switch_expr->cases[0].second->span, 1, 31, 1, 32);

        // Default value '2' is at 44 -> ends at 45
        assert_span(switch_expr->default_case->span, 1, 44, 1, 45);
    }

    TEST_F(AstSpanTest, ValidatesEnumDefinitionSpans) {
        auto ast = parse_code("enum Color: int { Red = 1, Blue = 2 }");

        ASSERT_EQ(ast->enum_definitions.size(), 1);
        auto enum_def = ast->enum_definitions[0].get();

        // 'enum' starts at 1, '}' is at 37 -> ends at 38
        assert_span(enum_def->span, 1, 1, 1, 38);

        // Underlying type 'int' starts at 13 -> ends at 16
        assert_span(enum_def->underlying_type->span, 1, 13, 1, 16);

        // First case value '1' is at 25 -> ends at 26
        assert_span(enum_def->cases[0].value->span, 1, 25, 1, 26);
    }

    TEST_F(AstSpanTest, ValidatesComplexDictLiteralAndNestedSpans) {
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
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assign_node, nullptr);

        auto dict = dynamic_cast<DictLiteral *>(assign_node->value.get());
        ASSERT_NE(dict, nullptr);
        ASSERT_EQ(dict->elements.size(), 8);

        // 1. Outer Dict bounds: '{' on line 1 col 15, '}' on line 10 col 1 -> ends at 2
        assert_span(dict->span, 1, 15, 10, 2);

        // 2. scalar: Unary Expression
        EXPECT_EQ(dict->elements[0].key, "scalar");
        auto scalar_val = dynamic_cast<UnaryExpression *>(dict->elements[0].value.get());
        ASSERT_NE(scalar_val, nullptr);
        assert_span(scalar_val->span, 2, 13, 2, 17); // '-100'

        // 3. equation: Binary Expression (grouped)
        EXPECT_EQ(dict->elements[1].key, "equation");
        auto eq_val = dynamic_cast<BinaryExpression *>(dict->elements[1].value.get());
        ASSERT_NE(eq_val, nullptr);
        EXPECT_EQ(eq_val->op, TokenType::Star);
        assert_span(eq_val->span, 3, 15, 3, 41); // '(base + 0.05) * multiplier'

        // 4. logic: Unary Expression
        EXPECT_EQ(dict->elements[2].key, "logic");
        auto logic_val = dynamic_cast<UnaryExpression *>(dict->elements[2].value.get());
        ASSERT_NE(logic_val, nullptr);
        EXPECT_EQ(logic_val->op, TokenType::Not);
        assert_span(logic_val->span, 4, 12, 4, 24); // 'not is_valid'

        // 5. group: Tuple Literal
        EXPECT_EQ(dict->elements[3].key, "group");
        auto group_val = dynamic_cast<TupleLiteral *>(dict->elements[3].value.get());
        ASSERT_NE(group_val, nullptr);
        assert_span(group_val->span, 5, 12, 5, 22); // '(1, a * b)'

        // 6. arr: Tensor Literal
        EXPECT_EQ(dict->elements[4].key, "arr");
        auto arr_val = dynamic_cast<TensorLiteral *>(dict->elements[4].value.get());
        ASSERT_NE(arr_val, nullptr);
        assert_span(arr_val->span, 6, 10, 6, 18); // '[10, 20]'

        // 7. subset: Bracket Access with slice
        EXPECT_EQ(dict->elements[5].key, "subset");
        auto subset_val = dynamic_cast<BracketAccess *>(dict->elements[5].value.get());
        ASSERT_NE(subset_val, nullptr);
        assert_span(subset_val->span, 7, 13, 7, 28); // 'history[0 : 10]'

        // 8. invoke: Function Call
        EXPECT_EQ(dict->elements[6].key, "invoke");
        auto invoke_val = dynamic_cast<FunctionCall *>(dict->elements[6].value.get());
        ASSERT_NE(invoke_val, nullptr);
        assert_span(invoke_val->span, 8, 13, 8, 34); // 'calc_risk(rate: 0.08)'

        // 9. nested: Dict Literal containing double-nested Unary Expression
        EXPECT_EQ(dict->elements[7].key, "nested");
        auto nested_val = dynamic_cast<DictLiteral *>(dict->elements[7].value.get());
        ASSERT_NE(nested_val, nullptr);
        assert_span(nested_val->span, 9, 13, 9, 36); // '{ inner: not not flag }'

        auto inner_val = dynamic_cast<UnaryExpression *>(nested_val->elements[0].value.get());
        ASSERT_NE(inner_val, nullptr);
        assert_span(inner_val->span, 9, 22, 9, 34); // 'not not flag'
    }

    TEST_F(AstSpanTest, ValidatesChainedAccessAndCallSpans) {
        auto ast = parse_code("let chain = user.get_roles()[0].name");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());

        // The outermost expression is the .name DotAccess
        auto final_dot = dynamic_cast<DotAccess *>(assign_node->value.get());
        ASSERT_NE(final_dot, nullptr);
        assert_span(final_dot->span, 1, 13, 1, 37); // 'user...name'

        // The target of .name is the [0] BracketAccess
        auto bracket_access = dynamic_cast<BracketAccess *>(final_dot->target.get());
        ASSERT_NE(bracket_access, nullptr);
        assert_span(bracket_access->span, 1, 13, 1, 32); // 'user.get_roles()[0]'

        // The target of [0] is the get_roles() FunctionCall
        auto func_call = dynamic_cast<FunctionCall *>(bracket_access->target.get());
        ASSERT_NE(func_call, nullptr);
        assert_span(func_call->span, 1, 13, 1, 29); // 'user.get_roles()'

        // The target of the function call is the .get_roles DotAccess
        auto method_dot = dynamic_cast<DotAccess *>(func_call->target.get());
        ASSERT_NE(method_dot, nullptr);
        assert_span(method_dot->span, 1, 13, 1, 27); // 'user.get_roles'

        // The base target is the identifier 'user'
        auto root_id = dynamic_cast<IdentifierAccess *>(method_dot->target.get());
        ASSERT_NE(root_id, nullptr);
        assert_span(root_id->span, 1, 13, 1, 17); // 'user'
    }

    TEST_F(AstSpanTest, ValidatesEmptyStructuresSpans) {
        std::string code =
                "let empty_arr = []\n"
                "let empty_dict = {}";

        auto ast = parse_code(code);

        // Empty tensor
        auto assign_arr = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        auto tensor = dynamic_cast<TensorLiteral *>(assign_arr->value.get());
        ASSERT_NE(tensor, nullptr);
        assert_span(tensor->span, 1, 17, 1, 19); // '[]'

        // Empty dict
        auto assign_dict = dynamic_cast<Assignment *>(ast->execution_steps[1].get());
        auto dict = dynamic_cast<DictLiteral *>(assign_dict->value.get());
        ASSERT_NE(dict, nullptr);
        assert_span(dict->span, 2, 18, 2, 20); // '{}'
    }

    TEST_F(AstSpanTest, ValidatesDeepTypeAnnotationSpans) {
        auto ast = parse_code("let matrix: matrix<int> = [[1]]");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());

        // Outermost type annotation 'matrix<int>'
        auto outer_type = assign_node->targets[0].second.get();
        ASSERT_NE(outer_type, nullptr);
        assert_span(outer_type->span, 1, 13, 1, 24);

        // The value '[[1]]'
        auto outer_tensor = dynamic_cast<TensorLiteral *>(assign_node->value.get());
        ASSERT_NE(outer_tensor, nullptr);
        assert_span(outer_tensor->span, 1, 27, 1, 32);
    }

    TEST_F(AstSpanTest, ValidatesGroupingSpan) {
        auto ast = parse_code("let a = (1 + 2) * 3");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());

        auto outer_mul = dynamic_cast<BinaryExpression *>(assign_node->value.get());
        ASSERT_NE(outer_mul, nullptr);
        assert_span(outer_mul->span, 1, 9, 1, 20);

        auto group = dynamic_cast<GroupingExpression *>(outer_mul->left.get());
        ASSERT_NE(group, nullptr);
        assert_span(group->span, 1, 9, 1, 16);
    }
}
