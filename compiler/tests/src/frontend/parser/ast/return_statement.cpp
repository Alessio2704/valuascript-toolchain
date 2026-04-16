#include "frontend/parser/helpers/ast_base_test.h"

namespace valuascript::compiler::test {
    TEST_F(AstBaseTest, ValidatesPrimitiveReturn) {
        // Proves a standard primitive literal is wrapped correctly.

        auto ast = parse_code("func get_rate() -> scalar { return 0.05 }");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);

        auto ret_stmt = dynamic_cast<ReturnStatement *>(func->body[0].get());
        ASSERT_NE(ret_stmt, nullptr);

        auto num_val = dynamic_cast<NumberLiteral *>(ret_stmt->values[0].get());
        ASSERT_NE(num_val, nullptr) << "Return value must be a NumberLiteral";
        EXPECT_EQ(num_val->value, "0.05");
    }

    TEST_F(AstBaseTest, ValidatesComplexMathAndFunctionCallReturn) {
        // Proves maximum orthogonality: the return statement successfully captures a highly nested expression tree.

        auto ast = parse_code("func calculate(wacc: scalar) -> scalar { return base_rate + get_premium(w: wacc) }");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);

        auto ret_stmt = dynamic_cast<ReturnStatement *>(func->body[0].get());
        ASSERT_NE(ret_stmt, nullptr);

        auto math_op = dynamic_cast<BinaryExpression *>(ret_stmt->values[0].get());
        ASSERT_NE(math_op, nullptr) << "Return value must be a BinaryExpression";
        EXPECT_EQ(math_op->op, TokenType::Plus);

        // Left side of math
        auto left_id = dynamic_cast<IdentifierAccess *>(math_op->left.get());
        ASSERT_NE(left_id, nullptr);
        EXPECT_EQ(left_id->name, "base_rate");

        // Right side of math: Nested Function Call with named arguments
        auto right_call = dynamic_cast<FunctionCall *>(math_op->right.get());
        ASSERT_NE(right_call, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(right_call->target.get())->name, "get_premium");

        ASSERT_EQ(right_call->arguments.size(), 1);
        EXPECT_EQ(right_call->arguments[0].first, "w");
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(right_call->arguments[0].second.get())->name, "wacc");
    }

    TEST_F(AstBaseTest, ValidatesComplexTupleReturn) {
        // Proves that parenthesis-wrapped comma-separated lists correctly parse into a TupleLiteral,
        // and that the elements inside retain perfect operator precedence and named-argument bindings.

        auto ast = parse_code("func get_data() -> (scalar, scalar, scalar) { return (1, a + b * func_call(x: 1), c) }");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);

        auto ret_stmt = dynamic_cast<ReturnStatement *>(func->body[0].get());
        ASSERT_NE(ret_stmt, nullptr);

        // 1. Verify the return value is a TupleLiteral
        auto tuple_val = dynamic_cast<TupleLiteral *>(ret_stmt->values[0].get());
        ASSERT_NE(tuple_val, nullptr) << "Return value must be a TupleLiteral for parenthesis groupings";
        ASSERT_EQ(tuple_val->elements.size(), 3) << "Tuple should contain exactly 3 elements";

        // ==========================================
        // ELEMENT 0: Primitive Number (1)
        // ==========================================
        auto elem0 = dynamic_cast<NumberLiteral *>(tuple_val->elements[0].get());
        ASSERT_NE(elem0, nullptr);
        EXPECT_EQ(elem0->value, "1");

        // ==========================================
        // ELEMENT 1: Complex Math (a + b * func_call(x: 1))
        // ==========================================
        // Because '*' has higher precedence than '+', the root of this element MUST be the '+'
        auto elem1_add = dynamic_cast<BinaryExpression *>(tuple_val->elements[1].get());
        ASSERT_NE(elem1_add, nullptr) << "Element 1 must be an addition operation at its root";
        EXPECT_EQ(elem1_add->op, TokenType::Plus);

        // Verify left side of '+' is 'a'
        auto a_id = dynamic_cast<IdentifierAccess *>(elem1_add->left.get());
        ASSERT_NE(a_id, nullptr);
        EXPECT_EQ(a_id->name, "a");

        // Verify right side of '+' is the multiplication (b * func_call)
        auto elem1_mult = dynamic_cast<BinaryExpression *>(elem1_add->right.get());
        ASSERT_NE(elem1_mult, nullptr) << "Right side of addition must be the multiplication operation";
        EXPECT_EQ(elem1_mult->op, TokenType::Star);

        // Verify left side of '*' is 'b'
        auto b_id = dynamic_cast<IdentifierAccess *>(elem1_mult->left.get());
        ASSERT_NE(b_id, nullptr);
        EXPECT_EQ(b_id->name, "b");

        // Verify right side of '*' is the function call
        auto f_call = dynamic_cast<FunctionCall *>(elem1_mult->right.get());
        ASSERT_NE(f_call, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(f_call->target.get())->name, "func_call");

        // Verify the function call's named argument (x: 1)
        ASSERT_EQ(f_call->arguments.size(), 1);
        EXPECT_EQ(f_call->arguments[0].first, "x");

        auto arg_val = dynamic_cast<NumberLiteral *>(f_call->arguments[0].second.get());
        ASSERT_NE(arg_val, nullptr);
        EXPECT_EQ(arg_val->value, "1");

        // ==========================================
        // ELEMENT 2: Identifier (c)
        // ==========================================
        auto elem2 = dynamic_cast<IdentifierAccess *>(tuple_val->elements[2].get());
        ASSERT_NE(elem2, nullptr);
        EXPECT_EQ(elem2->name, "c");
    }

    TEST_F(AstBaseTest, ValidatesMultipleReturnValues) {
        // Proves that the ReturnStatement natively supports multiple disjointed return values,
        // successfully isolating a TupleLiteral as the first return value and a NumberLiteral as the second.

        auto ast = parse_code("func get_mixed_data() -> something { return (1, 2), 3 }");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);

        auto ret_stmt = dynamic_cast<ReturnStatement *>(func->body[0].get());
        ASSERT_NE(ret_stmt, nullptr);

        // 1. Verify the ReturnStatement holds exactly 2 disjoint values
        ASSERT_EQ(ret_stmt->values.size(), 2) << "Return statement should hold exactly 2 distinct expressions";

        // ==========================================
        // RETURN VALUE 0: Tuple Literal (1, 2)
        // ==========================================
        auto ret_val_0 = dynamic_cast<TupleLiteral *>(ret_stmt->values[0].get());
        ASSERT_NE(ret_val_0, nullptr) << "First return value must be a TupleLiteral";
        ASSERT_EQ(ret_val_0->elements.size(), 2);

        auto tuple_elem_0 = dynamic_cast<NumberLiteral *>(ret_val_0->elements[0].get());
        ASSERT_NE(tuple_elem_0, nullptr);
        EXPECT_EQ(tuple_elem_0->value, "1");

        auto tuple_elem_1 = dynamic_cast<NumberLiteral *>(ret_val_0->elements[1].get());
        ASSERT_NE(tuple_elem_1, nullptr);
        EXPECT_EQ(tuple_elem_1->value, "2");

        // ==========================================
        // RETURN VALUE 1: Scalar (3)
        // ==========================================
        auto ret_val_1 = dynamic_cast<NumberLiteral *>(ret_stmt->values[1].get());
        ASSERT_NE(ret_val_1, nullptr) << "Second return value must be a standalone NumberLiteral";
        EXPECT_EQ(ret_val_1->value, "3");
    }
}
