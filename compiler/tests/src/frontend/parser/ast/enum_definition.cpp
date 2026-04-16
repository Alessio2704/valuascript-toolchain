#include "frontend/parser/helpers/ast_base_test.h"

namespace valuascript::compiler::test {
    TEST_F(AstBaseTest, ValidatesImplicitEnumDefinition) {
        auto ast = parse_code("enum Direction: string { UP, DOWN }");

        ASSERT_EQ(ast->enum_definitions.size(), 1);
        auto enum_def = ast->enum_definitions[0].get();

        // 1. Verify Header
        EXPECT_EQ(enum_def->name, "Direction");
        ASSERT_NE(enum_def->underlying_type, nullptr);
        EXPECT_EQ(enum_def->underlying_type->name, "string");

        // 2. Verify Cases
        ASSERT_EQ(enum_def->cases.size(), 2);

        EXPECT_EQ(enum_def->cases[0].name, "UP");
        EXPECT_EQ(enum_def->cases[0].value, nullptr) << "Implicit case must have a null expression pointer.";

        EXPECT_EQ(enum_def->cases[1].name, "DOWN");
        EXPECT_EQ(enum_def->cases[1].value, nullptr) << "Implicit case must have a null expression pointer.";
    }

    TEST_F(AstBaseTest, ValidatesMixedAndExplicitEnumDefinition) {
        auto ast = parse_code("enum StatusCode: integer { OK = 200, UNKNOWN, ERROR = 400 + 4 }");

        ASSERT_EQ(ast->enum_definitions.size(), 1);
        auto enum_def = ast->enum_definitions[0].get();

        EXPECT_EQ(enum_def->name, "StatusCode");
        EXPECT_EQ(enum_def->underlying_type->name, "integer");
        ASSERT_EQ(enum_def->cases.size(), 3);

        // Case 1: OK = 200 (Primitive Literal)
        EXPECT_EQ(enum_def->cases[0].name, "OK");
        auto ok_val = dynamic_cast<NumberLiteral *>(enum_def->cases[0].value.get());
        ASSERT_NE(ok_val, nullptr) << "Expected OK to have a NumberLiteral value.";
        EXPECT_EQ(ok_val->value, "200");

        // Case 2: UNKNOWN (Implicit)
        EXPECT_EQ(enum_def->cases[1].name, "UNKNOWN");
        EXPECT_EQ(enum_def->cases[1].value, nullptr) << "Expected UNKNOWN to be implicit (nullptr).";

        // Case 3: ERROR = 400 + 4 (Binary Expression)
        EXPECT_EQ(enum_def->cases[2].name, "ERROR");
        auto err_val = dynamic_cast<BinaryExpression *>(enum_def->cases[2].value.get());
        ASSERT_NE(err_val, nullptr) << "Expected ERROR to have a BinaryExpression value.";
        EXPECT_EQ(err_val->op, TokenType::Plus); // Or your specific plus token enum

        auto left_val = dynamic_cast<NumberLiteral *>(err_val->left.get());
        ASSERT_NE(left_val, nullptr);
        EXPECT_EQ(left_val->value, "400");

        auto right_val = dynamic_cast<NumberLiteral *>(err_val->right.get());
        ASSERT_NE(right_val, nullptr);
        EXPECT_EQ(right_val->value, "4");
    }

    TEST_F(AstBaseTest, ValidatesEnumWithComplexUnderlyingType) {
        auto ast = parse_code("enum State: Result<string, Error> { SUCCESS, FAILURE }");

        ASSERT_EQ(ast->enum_definitions.size(), 1);
        auto enum_def = ast->enum_definitions[0].get();

        EXPECT_EQ(enum_def->name, "State");

        // Verify the complex type geometry
        auto type = enum_def->underlying_type.get();
        ASSERT_NE(type, nullptr);
        EXPECT_EQ(type->name, "Result");
        ASSERT_EQ(type->generic_args.size(), 2);
        EXPECT_EQ(type->generic_args[0]->name, "string");
        EXPECT_EQ(type->generic_args[1]->name, "Error");

        ASSERT_EQ(enum_def->cases.size(), 2);
        EXPECT_EQ(enum_def->cases[0].name, "SUCCESS");
        EXPECT_EQ(enum_def->cases[1].name, "FAILURE");
    }

    TEST_F(AstBaseTest, ValidatesEmptyEnumDefinition) {
        // Proves the parser handles immediate brace closure without crashing

        auto ast = parse_code("enum Phantom: string {}");

        ASSERT_EQ(ast->enum_definitions.size(), 1);
        auto enum_def = ast->enum_definitions[0].get();

        EXPECT_EQ(enum_def->name, "Phantom");
        EXPECT_EQ(enum_def->underlying_type->name, "string");

        // The critical assertion: the cases vector is initialized but empty
        EXPECT_TRUE(enum_def->cases.empty());
    }

    TEST_F(AstBaseTest, ValidatesEnumWithDeepPostfixExpression) {
        // AST Shape of Value: BracketAccess( FunctionCall( DotAccess(sys, get_fallback) ), 0 )

        auto ast = parse_code("enum Config: string { DEFAULT = sys.get_fallback()[0] }");

        ASSERT_EQ(ast->enum_definitions.size(), 1);
        auto enum_def = ast->enum_definitions[0].get();

        ASSERT_EQ(enum_def->cases.size(), 1);
        EXPECT_EQ(enum_def->cases[0].name, "DEFAULT");

        // Root of the expression: [0]
        auto bracket_acc = dynamic_cast<BracketAccess *>(enum_def->cases[0].value.get());
        ASSERT_NE(bracket_acc, nullptr) << "Expected value to be a BracketAccess";

        auto index = dynamic_cast<NumberLiteral *>(bracket_acc->index.get());
        ASSERT_NE(index, nullptr);
        EXPECT_EQ(index->value, "0");

        // Target of [0] -> get_fallback()
        auto func_call = dynamic_cast<FunctionCall *>(bracket_acc->target.get());
        ASSERT_NE(func_call, nullptr) << "Expected target to be a FunctionCall";

        // Target of function call -> .get_fallback
        auto dot_acc = dynamic_cast<DotAccess *>(func_call->target.get());
        ASSERT_NE(dot_acc, nullptr) << "Expected target to be a DotAccess";
        EXPECT_EQ(dot_acc->property_name, "get_fallback");

        // Target of .get_fallback -> sys
        auto sys_id = dynamic_cast<IdentifierAccess *>(dot_acc->target.get());
        ASSERT_NE(sys_id, nullptr);
        EXPECT_EQ(sys_id->name, "sys");
    }

    TEST_F(AstBaseTest, ValidatesEnumUsageAsDotAccess) {
        // Proves that enum cases are seamlessly handled by the generalized DotAccess node without new parser logic.

        auto ast = parse_code("enum MyEnum: string { ONE } \n let a = my_func(a: MyEnum.ONE)");
        ASSERT_EQ(ast->execution_steps.size(), 1);

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);
        EXPECT_EQ(assignment->targets.size(), 1);
        EXPECT_EQ(assignment->targets[0].first, "a");

        // Verify the value is a FunctionCall
        auto func_call = dynamic_cast<FunctionCall *>(assignment->value.get());
        ASSERT_NE(func_call, nullptr);

        auto func_id = dynamic_cast<IdentifierAccess *>(func_call->target.get());
        ASSERT_NE(func_id, nullptr);
        EXPECT_EQ(func_id->name, "my_func");

        // Verify the named argument "a: MyEnum.ONE"
        ASSERT_EQ(func_call->arguments.size(), 1);
        EXPECT_EQ(func_call->arguments[0].first, "a");

        // Verify the argument value is parsed perfectly as a DotAccess
        auto dot_acc = dynamic_cast<DotAccess *>(func_call->arguments[0].second.get());
        ASSERT_NE(dot_acc, nullptr) << "Expected MyEnum.ONE to parse as a DotAccess";
        EXPECT_EQ(dot_acc->property_name, "ONE");

        // Verify the target of the dot access is "MyEnum"
        auto enum_id = dynamic_cast<IdentifierAccess *>(dot_acc->target.get());
        ASSERT_NE(enum_id, nullptr);
        EXPECT_EQ(enum_id->name, "MyEnum");
    }

    TEST_F(AstBaseTest, ValidatesEnumAsTypeAnnotation) {
        // Proves the type parser treats the enum name identically to built-in types or structs.

        auto ast = parse_code("let dir: Direction = Direction.UP");
        ASSERT_EQ(ast->execution_steps.size(), 1);

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assignment, nullptr);
        ASSERT_EQ(assignment->targets.size(), 1);

        // Verify the Type Annotation is "Direction"
        auto type_ann = assignment->targets[0].second.get();
        ASSERT_NE(type_ann, nullptr) << "Expected a type annotation.";
        EXPECT_EQ(type_ann->name, "Direction");

        // Verify the Value is the DotAccess "Direction.UP"
        auto dot_acc = dynamic_cast<DotAccess *>(assignment->value.get());
        ASSERT_NE(dot_acc, nullptr);
        EXPECT_EQ(dot_acc->property_name, "UP");

        auto target_id = dynamic_cast<IdentifierAccess *>(dot_acc->target.get());
        ASSERT_NE(target_id, nullptr);
        EXPECT_EQ(target_id->name, "Direction");
    }

    TEST_F(AstBaseTest, ValidatesEnumWithUnaryExpression) {
        // Proves unary operators work seamlessly in case values.

        auto ast = parse_code("enum Position: integer { SHORT = -1, FLAT = 0, LONG = 1 }");
        ASSERT_EQ(ast->enum_definitions.size(), 1);

        auto enum_def = ast->enum_definitions[0].get();
        ASSERT_EQ(enum_def->cases.size(), 3);

        // Check Case 0: SHORT = -1
        EXPECT_EQ(enum_def->cases[0].name, "SHORT");

        // The value should be a UnaryExpression
        auto unary_expr = dynamic_cast<UnaryExpression *>(enum_def->cases[0].value.get());
        ASSERT_NE(unary_expr, nullptr) << "Expected SHORT value to be a UnaryExpression.";
        EXPECT_EQ(unary_expr->op, TokenType::Minus);

        // The operand should be the number 1
        auto num_literal = dynamic_cast<NumberLiteral *>(unary_expr->right.get());
        ASSERT_NE(num_literal, nullptr);
        EXPECT_EQ(num_literal->value, "1");
    }

    TEST_F(AstBaseTest, ValidatesEnumWithVector) {
        auto ast = parse_code("enum Test: vector<integer> { SHORT = [-1, 0, 1], LONG = [1, 2, 3, 4] }");
        ASSERT_EQ(ast->enum_definitions.size(), 1);

        auto enum_def = ast->enum_definitions[0].get();
        ASSERT_EQ(enum_def->cases.size(), 2);

        EXPECT_EQ(enum_def->cases[0].name, "SHORT");
        EXPECT_EQ(enum_def->cases[1].name, "LONG");

        auto tensor_1 = dynamic_cast<TensorLiteral *>(enum_def->cases[0].value.get());
        ASSERT_NE(tensor_1, nullptr) << "Expected SHORT value to be a TensorLiteral.";
        EXPECT_EQ(tensor_1->elements.size(), 3);
        auto unary = dynamic_cast<UnaryExpression *>(tensor_1->elements[0].get());
        EXPECT_EQ(unary->op, TokenType::Minus);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(unary->right.get())->value, "1");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(tensor_1->elements[1].get())->value, "0");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(tensor_1->elements[2].get())->value, "1");

        auto tensor_2 = dynamic_cast<TensorLiteral *>(enum_def->cases[1].value.get());
        ASSERT_NE(tensor_2, nullptr) << "Expected LONG value to be a TensorLiteral.";
        EXPECT_EQ(tensor_2->elements.size(), 4);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(tensor_2->elements[0].get())->value, "1");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(tensor_2->elements[1].get())->value, "2");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(tensor_2->elements[2].get())->value, "3");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(tensor_2->elements[3].get())->value, "4");
    }

    TEST_F(AstBaseTest, ValidatesEnumWithTensor) {
        auto ast = parse_code(
            "enum Direction: vector<int> { ONE = [[1,2,3], [3,4]], TWO = [[5,6], [7,8], [1,1,1,1]] }");
        ASSERT_EQ(ast->enum_definitions.size(), 1);

        auto enum_def = ast->enum_definitions[0].get();
        ASSERT_EQ(enum_def->cases.size(), 2);

        EXPECT_EQ(enum_def->cases[0].name, "ONE");
        EXPECT_EQ(enum_def->cases[1].name, "TWO");

        auto tensor_1 = dynamic_cast<TensorLiteral *>(enum_def->cases[0].value.get());
        ASSERT_NE(tensor_1, nullptr) << "Expected ONE value to be a TensorLiteral.";
        EXPECT_EQ(tensor_1->elements.size(), 2);

        auto tensor_1_1 = dynamic_cast<TensorLiteral *>(tensor_1->elements[0].get());
        auto tensor_1_2 = dynamic_cast<TensorLiteral *>(tensor_1->elements[1].get());

        ASSERT_NE(tensor_1_1, nullptr) << "Expected value to be a TensorLiteral.";
        ASSERT_NE(tensor_1_2, nullptr) << "Expected value to be a TensorLiteral.";

        EXPECT_EQ(tensor_1_1->elements.size(), 3);
        EXPECT_EQ(tensor_1_2->elements.size(), 2);

        auto tensor_2 = dynamic_cast<TensorLiteral *>(enum_def->cases[1].value.get());
        ASSERT_NE(tensor_2, nullptr) << "Expected TWO value to be a TensorLiteral.";
        EXPECT_EQ(tensor_2->elements.size(), 3);

        auto tensor_2_1 = dynamic_cast<TensorLiteral *>(tensor_2->elements[0].get());
        auto tensor_2_2 = dynamic_cast<TensorLiteral *>(tensor_2->elements[1].get());
        auto tensor_2_3 = dynamic_cast<TensorLiteral *>(tensor_2->elements[2].get());

        ASSERT_NE(tensor_2_1, nullptr) << "Expected value to be a TensorLiteral.";
        ASSERT_NE(tensor_2_2, nullptr) << "Expected value to be a TensorLiteral.";
        ASSERT_NE(tensor_2_3, nullptr) << "Expected value to be a TensorLiteral.";

        EXPECT_EQ(tensor_2_1->elements.size(), 2);
        EXPECT_EQ(tensor_2_2->elements.size(), 2);
        EXPECT_EQ(tensor_2_3->elements.size(), 4);
    }
}
