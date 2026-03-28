#include "../ast_base_test.h"
using namespace valuascript::compiler::test;

TEST_F(AstBaseTest, ValidatesEmptyTuple) {
    // Proves the prefix parser correctly intercepts empty parentheses and builds a 0-element tuple.

    auto ast = parse_code("let empty = ()");
    auto tuple_val = dynamic_cast<TupleLiteral *>(get_assigned_value(ast));

    ASSERT_NE(tuple_val, nullptr) << "Assigned value must be a TupleLiteral";
    EXPECT_EQ(tuple_val->elements.size(), 0) << "Empty tuple must have 0 elements";
}

TEST_F(AstBaseTest, ValidatesSingleElementInParenthesisIsNotATupleLiteral) {

    auto ast = parse_code("let single = (1)");
    auto tuple_val = dynamic_cast<TupleLiteral *>(get_assigned_value(ast));

    ASSERT_EQ(tuple_val, nullptr) << "Assigned value must not be a TupleLiteral";

    auto value = dynamic_cast<NumberLiteral*>(unwrap(get_assigned_value(ast)));
    ASSERT_EQ(value->value, "1");
}

TEST_F(AstBaseTest, ValidatesFlatHeterogeneousTuple) {
    // Proves a tuple can hold entirely different primitive shapes side-by-side.

    auto ast = parse_code("let data = (1, \"AAPL\", base_rate)");
    auto tuple_val = dynamic_cast<TupleLiteral *>(get_assigned_value(ast));

    ASSERT_NE(tuple_val, nullptr);
    ASSERT_EQ(tuple_val->elements.size(), 3);

    // Element 0: NumberLiteral
    auto elem0 = dynamic_cast<NumberLiteral *>(tuple_val->elements[0].get());
    ASSERT_NE(elem0, nullptr);
    EXPECT_EQ(elem0->value, "1");

    // Element 1: StringLiteral (assuming quote-stripping is implemented)
    auto elem1 = dynamic_cast<StringLiteral *>(tuple_val->elements[1].get());
    ASSERT_NE(elem1, nullptr);
    EXPECT_EQ(elem1->value, "\"AAPL\"");

    // Element 2: IdentifierAccess
    auto elem2 = dynamic_cast<IdentifierAccess *>(tuple_val->elements[2].get());
    ASSERT_NE(elem2, nullptr);
    EXPECT_EQ(elem2->name, "base_rate");
}

TEST_F(AstBaseTest, ValidatesNestedTuples) {
    // Proves that tuples can geometrically hold other tuples without pointer collapse.

    auto ast = parse_code("let nested = ((1, 2), (3, 4))");
    auto root_tuple = dynamic_cast<TupleLiteral *>(get_assigned_value(ast));

    ASSERT_NE(root_tuple, nullptr);
    ASSERT_EQ(root_tuple->elements.size(), 2) << "Root tuple should hold exactly 2 elements";

    // Nested Tuple 0: (1, 2)
    auto inner0 = dynamic_cast<TupleLiteral *>(root_tuple->elements[0].get());
    ASSERT_NE(inner0, nullptr) << "First element must be a nested TupleLiteral";
    ASSERT_EQ(inner0->elements.size(), 2);
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner0->elements[0].get())->value, "1");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner0->elements[1].get())->value, "2");

    // Nested Tuple 1: (3, 4)
    auto inner1 = dynamic_cast<TupleLiteral *>(root_tuple->elements[1].get());
    ASSERT_NE(inner1, nullptr) << "Second element must be a nested TupleLiteral";
    ASSERT_EQ(inner1->elements.size(), 2);
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner1->elements[0].get())->value, "3");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner1->elements[1].get())->value, "4");
}

TEST_F(AstBaseTest, ValidatesTupleWithComplexExpressions) {
    // Proves tuples correctly delegate inner math operations and function calls down the tree.

    auto ast = parse_code("let complex = (wacc * 100, fetch_data(ticker: \"MSFT\"))");
    auto tuple_val = dynamic_cast<TupleLiteral *>(get_assigned_value(ast));

    ASSERT_NE(tuple_val, nullptr);
    ASSERT_EQ(tuple_val->elements.size(), 2);

    // Element 0: Binary Expression
    auto math_op = dynamic_cast<BinaryExpression *>(tuple_val->elements[0].get());
    ASSERT_NE(math_op, nullptr) << "First element must be a BinaryExpression";
    EXPECT_EQ(math_op->op, TokenType::Star);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(math_op->left.get())->name, "wacc");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(math_op->right.get())->value, "100");

    // Element 1: Function Call
    auto func_call = dynamic_cast<FunctionCall *>(tuple_val->elements[1].get());
    ASSERT_NE(func_call, nullptr) << "Second element must be a FunctionCall";
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(func_call->target.get())->name, "fetch_data");

    ASSERT_EQ(func_call->arguments.size(), 1);
    EXPECT_EQ(func_call->arguments[0].first, "ticker");
    EXPECT_EQ(dynamic_cast<StringLiteral*>(func_call->arguments[0].second.get())->value, "\"MSFT\"");
}

TEST_F(AstBaseTest, ValidatesTupleVsGroupingDistinction) {
    // Proves the parser correctly distinguishes between mathematical grouping parentheses
    // and nested tuple parentheses within the exact same AST level.

    auto ast = parse_code("let mixed = (1, (1 + 3), (1, 2, 3))");
    auto root_tuple = dynamic_cast<TupleLiteral*>(get_assigned_value(ast));

    ASSERT_NE(root_tuple, nullptr);
    ASSERT_EQ(root_tuple->elements.size(), 3) << "Root tuple should hold exactly 3 elements";

    // ==========================================
    // ELEMENT 0: Primitive Number (1)
    // ==========================================
    auto elem0 = dynamic_cast<NumberLiteral*>(root_tuple->elements[0].get());
    ASSERT_NE(elem0, nullptr) << "Element 0 must be a NumberLiteral";
    EXPECT_EQ(elem0->value, "1");

    // ==========================================
    // ELEMENT 1: Mathematical Grouping (1 + 3)
    // ==========================================
    // The parser must completely discard the parentheses here and just return the BinaryExpression.
    // If it incorrectly wrapped it in a TupleLiteral, this dynamic_cast will fail.
    auto elem1_math = dynamic_cast<BinaryExpression*>(unwrap(root_tuple->elements[1].get()));
    ASSERT_NE(elem1_math, nullptr) << "Element 1 must be a pure BinaryExpression, NOT a TupleLiteral";
    EXPECT_EQ(elem1_math->op, TokenType::Plus);

    EXPECT_EQ(dynamic_cast<NumberLiteral*>(elem1_math->left.get())->value, "1");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(elem1_math->right.get())->value, "3");

    // ==========================================
    // ELEMENT 2: Nested Tuple (1, 2, 3)
    // ==========================================
    // Because of the commas, the parser must build a TupleLiteral here.
    auto elem2_tuple = dynamic_cast<TupleLiteral*>(root_tuple->elements[2].get());
    ASSERT_NE(elem2_tuple, nullptr) << "Element 2 must be a nested TupleLiteral";
    ASSERT_EQ(elem2_tuple->elements.size(), 3);

    EXPECT_EQ(dynamic_cast<NumberLiteral*>(elem2_tuple->elements[0].get())->value, "1");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(elem2_tuple->elements[1].get())->value, "2");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(elem2_tuple->elements[2].get())->value, "3");
}

TEST_F(AstBaseTest, ValidatesTupleWithDeepMathPrecedence) {
    // Proves that redundant grouping parentheses are stripped, but necessary
    // grouping parentheses correctly override standard operator precedence.

    auto ast = parse_code("let deep_math = (1, ((1 + 3) * 4), (1, 2, 3))");
    auto root_tuple = dynamic_cast<TupleLiteral*>(get_assigned_value(ast));

    ASSERT_NE(root_tuple, nullptr);
    ASSERT_EQ(root_tuple->elements.size(), 3) << "Root tuple should hold exactly 3 elements";

    // ==========================================
    // ELEMENT 0: Primitive Number (1)
    // ==========================================
    auto elem0 = dynamic_cast<NumberLiteral*>(root_tuple->elements[0].get());
    ASSERT_NE(elem0, nullptr);
    EXPECT_EQ(elem0->value, "1");

    // ==========================================
    // ELEMENT 1: Precedence-Overridden Math (((1 + 3) * 4))
    // ==========================================
    // 1. The outermost parentheses must be gone. The root of this element MUST be the '*'.
    auto elem1_mult = dynamic_cast<BinaryExpression*>(unwrap(root_tuple->elements[1].get()));
    ASSERT_NE(elem1_mult, nullptr) << "Element 1 must be a BinaryExpression (Multiplication)";
    EXPECT_EQ(elem1_mult->op, TokenType::Star);

    // 2. The left side of the '*' MUST be the '+' because of the (1 + 3) override.
    auto math_left_add = dynamic_cast<BinaryExpression*>(unwrap(elem1_mult->left.get()));
    ASSERT_NE(math_left_add, nullptr) << "Left side of multiplication must be the addition operation";
    EXPECT_EQ(math_left_add->op, TokenType::Plus);

    EXPECT_EQ(dynamic_cast<NumberLiteral*>(math_left_add->left.get())->value, "1");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(math_left_add->right.get())->value, "3");

    // 3. The right side of the '*' MUST be the '4'.
    auto math_right_val = dynamic_cast<NumberLiteral*>(elem1_mult->right.get());
    ASSERT_NE(math_right_val, nullptr);
    EXPECT_EQ(math_right_val->value, "4");

    // ==========================================
    // ELEMENT 2: Nested Tuple (1, 2, 3)
    // ==========================================
    auto elem2_tuple = dynamic_cast<TupleLiteral*>(root_tuple->elements[2].get());
    ASSERT_NE(elem2_tuple, nullptr) << "Element 2 must be a nested TupleLiteral";
    ASSERT_EQ(elem2_tuple->elements.size(), 3);

    EXPECT_EQ(dynamic_cast<NumberLiteral*>(elem2_tuple->elements[0].get())->value, "1");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(elem2_tuple->elements[1].get())->value, "2");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(elem2_tuple->elements[2].get())->value, "3");
}