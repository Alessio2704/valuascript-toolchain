#include "frontend/parser/ast_base_test.h"

namespace valuascript::compiler::test {
    TEST_F(AstBaseTest, Validates1DVector) {
        auto ast = parse_code("let v = [100, 110, 121]");
        auto vec = get_assigned_vector(ast);
        ASSERT_NE(vec, nullptr);
        ASSERT_EQ(vec->elements.size(), 3);

        auto elem0 = dynamic_cast<NumberLiteral *>(vec->elements[0].get());
        auto elem1 = dynamic_cast<NumberLiteral *>(vec->elements[1].get());
        auto elem2 = dynamic_cast<NumberLiteral *>(vec->elements[2].get());

        ASSERT_NE(elem0, nullptr);
        ASSERT_NE(elem1, nullptr);
        ASSERT_NE(elem2, nullptr);

        EXPECT_EQ(elem0->value, "100");
        EXPECT_EQ(elem1->value, "110");
        EXPECT_EQ(elem2->value, "121");
    }

    TEST_F(AstBaseTest, Validates2DMatrix) {
        auto ast = parse_code("let m = [[1, 2], [3, 4]]");
        auto matrix = get_assigned_vector(ast);
        ASSERT_NE(matrix, nullptr);
        ASSERT_EQ(matrix->elements.size(), 2) << "Matrix should have 2 rows";

        // Check Row 0: [1, 2]
        auto row0 = dynamic_cast<TensorLiteral *>(matrix->elements[0].get());
        ASSERT_NE(row0, nullptr) << "Element 0 must be a nested TensorLiteral";
        ASSERT_EQ(row0->elements.size(), 2);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(row0->elements[0].get())->value, "1");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(row0->elements[1].get())->value, "2");

        // Check Row 1: [3, 4]
        auto row1 = dynamic_cast<TensorLiteral *>(matrix->elements[1].get());
        ASSERT_NE(row1, nullptr) << "Element 1 must be a nested TensorLiteral";
        ASSERT_EQ(row1->elements.size(), 2);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(row1->elements[0].get())->value, "3");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(row1->elements[1].get())->value, "4");
    }

    TEST_F(AstBaseTest, Validates3DTensor) {
        auto ast = parse_code("let t = [[[1], [2]], [[3], [4]]]");

        auto tensor3d = get_assigned_vector(ast);
        ASSERT_NE(tensor3d, nullptr);
        ASSERT_EQ(tensor3d->elements.size(), 2) << "Root tensor should have 2 matrices";

        // Dive into: Matrix 0 -> Row 1 -> Column 0 (The value '2')
        auto matrix0 = dynamic_cast<TensorLiteral *>(tensor3d->elements[0].get());
        ASSERT_NE(matrix0, nullptr);

        auto row1 = dynamic_cast<TensorLiteral *>(matrix0->elements[1].get());
        ASSERT_NE(row1, nullptr);

        auto val = dynamic_cast<NumberLiteral *>(row1->elements[0].get());
        ASSERT_NE(val, nullptr);
        EXPECT_EQ(val->value, "2") << "Deep geometry failed to map to the correct value";
    }

    TEST_F(AstBaseTest, ValidatesOrthogonalityInsideVectors) {
        // Proves that the parser doesn't just expect numbers, but any valid expression shape.
        auto ast = parse_code("let mixed = [wacc * 100, get_fcf(), [1, 2][0]]");
        auto mixed = get_assigned_vector(ast);
        ASSERT_NE(mixed, nullptr);
        ASSERT_EQ(mixed->elements.size(), 3);

        // Element 0: Math Expression (wacc * 100)
        auto math_expr = dynamic_cast<BinaryExpression *>(mixed->elements[0].get());
        ASSERT_NE(math_expr, nullptr);
        EXPECT_EQ(math_expr->op, TokenType::Star);

        // Element 1: Function Call (get_fcf())
        auto func_call = dynamic_cast<FunctionCall *>(mixed->elements[1].get());
        ASSERT_NE(func_call, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(func_call->target.get())->name, "get_fcf");

        // Element 2: Inline Vector Access ([1, 2][0])
        auto vec_access = dynamic_cast<BracketAccess *>(mixed->elements[2].get());
        ASSERT_NE(vec_access, nullptr);
        auto inline_vec = dynamic_cast<TensorLiteral *>(vec_access->target.get());
        ASSERT_NE(inline_vec, nullptr);
        EXPECT_EQ(inline_vec->elements.size(), 2);
    }

    TEST_F(AstBaseTest, ValidatesEmptyVectors) {
        // Edge case: must not crash when no elements are present.
        auto ast = parse_code("let empty = []");
        auto empty_vec = get_assigned_vector(ast);
        ASSERT_NE(empty_vec, nullptr);
        EXPECT_EQ(empty_vec->elements.size(), 0);
    }
}
