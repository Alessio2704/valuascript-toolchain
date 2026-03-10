#include "../ast_base_test.h"
using namespace valuascript::compiler::test;

TEST_F(AstBaseTest, ValidatesDeepIdentifierAccess) {
    // Proves the infinite while(true) loop in parse_atom correctly chains left-to-right.
    // The outermost node must be the [2] access.

    auto ast = parse_code("let val = matrix[0][1][2]");
    auto access_2 = dynamic_cast<BracketAccess*>(get_assigned_value(ast));
    ASSERT_NE(access_2, nullptr);
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(access_2->index.get())->value, "2");

    auto access_1 = dynamic_cast<BracketAccess*>(access_2->target.get());
    ASSERT_NE(access_1, nullptr);
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(access_1->index.get())->value, "1");

    auto access_0 = dynamic_cast<BracketAccess*>(access_1->target.get());
    ASSERT_NE(access_0, nullptr);
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(access_0->index.get())->value, "0");

    auto target_id = dynamic_cast<IdentifierAccess*>(access_0->target.get());
    ASSERT_NE(target_id, nullptr);
    EXPECT_EQ(target_id->name, "matrix");
}

TEST_F(AstBaseTest, ValidatesFunctionCallAsTarget) {
    // Proves we can immediately index into the return value of a function.

    auto ast = parse_code("let val = get_projections(a: ticker)[5]");
    auto access = dynamic_cast<BracketAccess*>(get_assigned_value(ast));
    ASSERT_NE(access, nullptr);
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(access->index.get())->value, "5");

    auto func_call = dynamic_cast<FunctionCall*>(access->target.get());
    ASSERT_NE(func_call, nullptr) << "Target of access should be a FunctionCall";
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(func_call->target.get())->name, "get_projections");
    ASSERT_EQ(func_call->arguments.size(), 1);
}

TEST_F(AstBaseTest, ValidatesParenthesizedBinaryOperationAsTarget) {
    // Proves that grouping parentheses correctly wrap a binary operation into a valid target.

    auto ast = parse_code("let val = (base_case + bull_case)[0]");
    auto access = dynamic_cast<BracketAccess*>(get_assigned_value(ast));
    ASSERT_NE(access, nullptr);

    auto bin_op = dynamic_cast<BinaryExpression*>(access->target.get());
    ASSERT_NE(bin_op, nullptr) << "Target of access should be a BinaryExpression (due to parentheses)";
    EXPECT_EQ(bin_op->op, TokenType::Plus);

    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(bin_op->left.get())->name, "base_case");
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(bin_op->right.get())->name, "bull_case");
}

TEST_F(AstBaseTest, ValidatesUnaryPrecedenceWithAccess) {
    // CRITICAL PRECEDENCE TEST: Postfix [] is higher than Prefix -.
    // The AST root must be Unary(-), and its child must be BracketAccess([0]).

    auto ast = parse_code("let val = -matrix[0]");
    auto root_val = get_assigned_value(ast);

    auto unary_op = dynamic_cast<UnaryExpression*>(root_val);
    ASSERT_NE(unary_op, nullptr) << "Root node must be UnaryExpression, NOT BracketAccess";
    EXPECT_EQ(unary_op->op, TokenType::Minus);

    auto access = dynamic_cast<BracketAccess*>(unary_op->right.get());
    ASSERT_NE(access, nullptr) << "The target of the unary minus must be the BracketAccess";

    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(access->target.get())->name, "matrix");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(access->index.get())->value, "0");
}

TEST_F(AstBaseTest, ValidatesComplexExpressionAsIndex) {
    // Proves the index itself can be a highly complex, nested expression.

    auto ast = parse_code("let val = data[get_offset() + 2]");
    auto access = dynamic_cast<BracketAccess*>(get_assigned_value(ast));
    ASSERT_NE(access, nullptr);

    auto index_bin_op = dynamic_cast<BinaryExpression*>(access->index.get());
    ASSERT_NE(index_bin_op, nullptr) << "Index should be a BinaryExpression";
    EXPECT_EQ(index_bin_op->op, TokenType::Plus);

    auto func_call = dynamic_cast<FunctionCall*>(index_bin_op->left.get());
    ASSERT_NE(func_call, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(func_call->target.get())->name, "get_offset");

    EXPECT_EQ(dynamic_cast<NumberLiteral*>(index_bin_op->right.get())->value, "2");
}

TEST_F(AstBaseTest, ValidatesExactTargetChainingForMultiDimensionalAccess) {
    // This test forensically unpacks the memory pointers to prove that
    // the target of [15] is the [10] access, the target of [10] is the [5] access,
    // and the target of [5] is the base identifier "tensor".

    auto ast = parse_code("let val = tensor[5][10][15]");

    // get_assigned_value() returns the raw Expression* on the right side of the '='
    auto root_access = dynamic_cast<BracketAccess*>(get_assigned_value(ast));

    // ==========================================
    // LAYER 3 (Outermost): [15]
    // ==========================================
    ASSERT_NE(root_access, nullptr) << "Root must be the outermost BracketAccess";
    auto index3 = dynamic_cast<NumberLiteral*>(root_access->index.get());
    ASSERT_NE(index3, nullptr) << "Outermost index must be a NumberLiteral";
    EXPECT_EQ(index3->value, "15");

    // ==========================================
    // LAYER 2 (Middle): [10]
    // ==========================================
    // The target of the [15] access MUST be another BracketAccess
    auto layer2_target = dynamic_cast<BracketAccess*>(root_access->target.get());
    ASSERT_NE(layer2_target, nullptr) << "Target of [15] must be the [10] BracketAccess node";

    auto index2 = dynamic_cast<NumberLiteral*>(layer2_target->index.get());
    ASSERT_NE(index2, nullptr) << "Middle index must be a NumberLiteral";
    EXPECT_EQ(index2->value, "10");

    // ==========================================
    // LAYER 1 (Innermost): [5]
    // ==========================================
    // The target of the [10] access MUST be another BracketAccess
    auto layer1_target = dynamic_cast<BracketAccess*>(layer2_target->target.get());
    ASSERT_NE(layer1_target, nullptr) << "Target of [10] must be the [5] BracketAccess node";

    auto index1 = dynamic_cast<NumberLiteral*>(layer1_target->index.get());
    ASSERT_NE(index1, nullptr) << "Innermost index must be a NumberLiteral";
    EXPECT_EQ(index1->value, "5");

    // ==========================================
    // BASE LAYER (The Identifier)
    // ==========================================
    // The target of the [5] access MUST be the raw IdentifierAccess
    auto base_target = dynamic_cast<IdentifierAccess*>(layer1_target->target.get());
    ASSERT_NE(base_target, nullptr) << "Target of [5] must be the base identifier";
    EXPECT_EQ(base_target->name, "tensor");
}

TEST_F(AstBaseTest, ValidatesDeepTensorSliceAccess) {
    // Proves that the colon operator correctly binds as a BinaryExpression
    // INSIDE the BracketAccess index, and respects math precedence on its right side.

    auto ast = parse_code("let slice = matrix[1 : limit - 1]");

    // 1. Get the BracketAccess node
    auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(assign, nullptr);
    auto access = dynamic_cast<BracketAccess*>(assign->value.get());
    ASSERT_NE(access, nullptr) << "Right side of assignment must be a BracketAccess";

    // 2. Verify the Target
    auto target = dynamic_cast<IdentifierAccess*>(access->target.get());
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->name, "matrix");

    // 3. Verify the Index (The Slice Range)
    auto slice_op = dynamic_cast<BinaryExpression*>(access->index.get());
    ASSERT_NE(slice_op, nullptr) << "Index must be a BinaryExpression representing the slice";
    EXPECT_EQ(slice_op->op, TokenType::Colon) << "Slice operator must be a colon";

    // 4. Verify Slice Start (Left side of colon)
    auto start_node = dynamic_cast<NumberLiteral*>(slice_op->left.get());
    ASSERT_NE(start_node, nullptr);
    EXPECT_EQ(start_node->value, "1");

    // 5. Verify Slice End (Right side of colon, which is a math expression)
    auto end_node = dynamic_cast<BinaryExpression*>(slice_op->right.get());
    ASSERT_NE(end_node, nullptr) << "Right side of slice must be the 'limit - 1' BinaryExpression";
    EXPECT_EQ(end_node->op, TokenType::Minus);

    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(end_node->left.get())->name, "limit");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(end_node->right.get())->value, "1");
}

TEST_F(AstBaseTest, ValidatesSliceMissingLeftBound) {
    // Proves the parser correctly assigns nullptr to the left side of the colon.

    auto ast = parse_code("let slice = matrix[: 5]");

    auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(assign, nullptr);
    auto access = dynamic_cast<BracketAccess*>(assign->value.get());
    ASSERT_NE(access, nullptr);

    auto slice_op = dynamic_cast<BinaryExpression*>(access->index.get());
    ASSERT_NE(slice_op, nullptr) << "Index must be a BinaryExpression (slice)";
    EXPECT_EQ(slice_op->op, TokenType::Colon);

    // 1. Verify Left is Null (Implicit Start)
    EXPECT_EQ(slice_op->left.get(), nullptr) << "Left bound must be implicitly null";

    // 2. Verify Right is parsed correctly
    auto right_node = dynamic_cast<NumberLiteral*>(slice_op->right.get());
    ASSERT_NE(right_node, nullptr);
    EXPECT_EQ(right_node->value, "5");
}

TEST_F(AstBaseTest, ValidatesSliceMissingRightBound) {
    // Proves the parser correctly assigns nullptr to the right side of the colon.

    auto ast = parse_code("let slice = matrix[1 :]");

    auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(assign, nullptr);
    auto access = dynamic_cast<BracketAccess*>(assign->value.get());
    ASSERT_NE(access, nullptr);

    auto slice_op = dynamic_cast<BinaryExpression*>(access->index.get());
    ASSERT_NE(slice_op, nullptr) << "Index must be a BinaryExpression (slice)";
    EXPECT_EQ(slice_op->op, TokenType::Colon);

    // 1. Verify Left is parsed correctly
    auto left_node = dynamic_cast<NumberLiteral*>(slice_op->left.get());
    ASSERT_NE(left_node, nullptr);
    EXPECT_EQ(left_node->value, "1");

    // 2. Verify Right is Null (Implicit End)
    EXPECT_EQ(slice_op->right.get(), nullptr) << "Right bound must be implicitly null";
}

TEST_F(AstBaseTest, ValidatesSliceMissingBothBounds) {
    // Proves a full tensor clone operation results in double nullptrs.

    auto ast = parse_code("let clone = matrix[:]");

    auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(assign, nullptr);
    auto access = dynamic_cast<BracketAccess*>(assign->value.get());
    ASSERT_NE(access, nullptr);

    auto slice_op = dynamic_cast<BinaryExpression*>(access->index.get());
    ASSERT_NE(slice_op, nullptr) << "Index must be a BinaryExpression (slice)";
    EXPECT_EQ(slice_op->op, TokenType::Colon);

    // 1. Verify Left is Null
    EXPECT_EQ(slice_op->left.get(), nullptr) << "Left bound must be implicitly null";

    // 2. Verify Right is Null
    EXPECT_EQ(slice_op->right.get(), nullptr) << "Right bound must be implicitly null";
}

TEST_F(AstBaseTest, ValidatesBracketAccessOnTupleLiteral) {
    // Proves that the target of a BracketAccess can be a raw TupleLiteral expression.

    auto ast = parse_code("let val = (10, 20)[0]");
    ASSERT_EQ(ast->execution_steps.size(), 1);

    auto assignment = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(assignment, nullptr);

    // 1. Root: [0]
    auto bracket_access = dynamic_cast<BracketAccess*>(assignment->value.get());
    ASSERT_NE(bracket_access, nullptr) << "Root expression must be a BracketAccess";

    // 2. Index: 0
    auto index = dynamic_cast<NumberLiteral*>(bracket_access->index.get());
    ASSERT_NE(index, nullptr);
    EXPECT_EQ(index->value, "0");

    // 3. Target: (10, 20)
    auto tuple_target = dynamic_cast<TupleLiteral*>(bracket_access->target.get());
    ASSERT_NE(tuple_target, nullptr) << "Target of bracket access must be a TupleLiteral";
    ASSERT_EQ(tuple_target->elements.size(), 2);

    auto first_element = dynamic_cast<NumberLiteral*>(tuple_target->elements[0].get());
    ASSERT_NE(first_element, nullptr);
    EXPECT_EQ(first_element->value, "10");

    auto second_element = dynamic_cast<NumberLiteral*>(tuple_target->elements[1].get());
    ASSERT_NE(second_element, nullptr);
    EXPECT_EQ(second_element->value, "20");
}

TEST_F(AstBaseTest, ValidatesBracketAccessOnFunctionCall) {
    // Proves that BracketAccess seamlessly chains off a FunctionCall.

    auto ast = parse_code("let val = get_pair()[1]");
    ASSERT_EQ(ast->execution_steps.size(), 1);

    auto assignment = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(assignment, nullptr);

    // 1. Root: [1]
    auto bracket_access = dynamic_cast<BracketAccess*>(assignment->value.get());
    ASSERT_NE(bracket_access, nullptr) << "Root expression must be a BracketAccess";

    // 2. Index: 1
    auto index = dynamic_cast<NumberLiteral*>(bracket_access->index.get());
    ASSERT_NE(index, nullptr);
    EXPECT_EQ(index->value, "1");

    // 3. Target: get_pair()
    auto func_call = dynamic_cast<FunctionCall*>(bracket_access->target.get());
    ASSERT_NE(func_call, nullptr) << "Target of bracket access must be a FunctionCall";

    auto func_target = dynamic_cast<IdentifierAccess*>(func_call->target.get());
    ASSERT_NE(func_target, nullptr);
    EXPECT_EQ(func_target->name, "get_pair");
}

TEST_F(AstBaseTest, ValidatesMixedBracketAndDotAccess) {
    // AST Shape: DotAccess( BracketAccess( FunctionCall( DotAccess(model, fetch_bounds) ), 1 ), max )

    auto ast = parse_code("let val = model.fetch_bounds()[1].max");
    ASSERT_EQ(ast->execution_steps.size(), 1);

    auto assignment = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(assignment, nullptr);

    // 1. Root: .max
    auto root_access = dynamic_cast<DotAccess*>(assignment->value.get());
    ASSERT_NE(root_access, nullptr);
    EXPECT_EQ(root_access->property_name, "max");

    // 2. Target of .max -> [1]
    auto bracket_access = dynamic_cast<BracketAccess*>(root_access->target.get());
    ASSERT_NE(bracket_access, nullptr);

    auto index = dynamic_cast<NumberLiteral*>(bracket_access->index.get());
    ASSERT_NE(index, nullptr);
    EXPECT_EQ(index->value, "1");

    // 3. Target of [1] -> fetch_bounds()
    auto func_call = dynamic_cast<FunctionCall*>(bracket_access->target.get());
    ASSERT_NE(func_call, nullptr);

    // 4. Target of function call -> .fetch_bounds
    auto fetch_access = dynamic_cast<DotAccess*>(func_call->target.get());
    ASSERT_NE(fetch_access, nullptr);
    EXPECT_EQ(fetch_access->property_name, "fetch_bounds");

    // 5. Target of .fetch_bounds -> model
    auto model_id = dynamic_cast<IdentifierAccess*>(fetch_access->target.get());
    ASSERT_NE(model_id, nullptr);
    EXPECT_EQ(model_id->name, "model");
}