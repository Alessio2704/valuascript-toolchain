#include "frontend/parser/helpers/ast_base_test.h"

namespace valuascript::compiler::test
{
    TEST_F(AstBaseTest, ValidatesSignatureAndDocstring)
    {
        auto ast = parse_code(R"(func calculate(x: scalar, y: boolean) -> (scalar, scalar) { """Computes values""" })");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);

        // 1. Function Name
        EXPECT_EQ(func->name, "calculate");

        // 2. Docstring
        ASSERT_TRUE(func->docstring.has_value());
        EXPECT_EQ(func->docstring.value(), "\"\"\"Computes values\"\"\"");

        // 3. Parameters
        ASSERT_EQ(func->parameters.size(), 2);

        EXPECT_EQ(func->parameters[0].name, "x");
        EXPECT_EQ(func->parameters[0].type->name, "scalar");
        EXPECT_TRUE(func->parameters[0].type->generic_args.empty());

        EXPECT_EQ(func->parameters[1].name, "y");
        EXPECT_EQ(func->parameters[1].type->name, "boolean");

        // 4. Return Types (Tuple)
        ASSERT_EQ(func->return_types.size(), 1);

        auto tuple_return_annotation = dynamic_cast<TupleTypeAnnotation*>(func->return_types[0].get());

        ASSERT_EQ(tuple_return_annotation->element_types.size(), 2);
        ASSERT_EQ(tuple_return_annotation->element_types[0]->name, "scalar");
        ASSERT_EQ(tuple_return_annotation->element_types[1]->name, "scalar");

        // Body should be empty (docstring is not a statement)
        EXPECT_EQ(func->body.size(), 0);
    }

    TEST_F(AstBaseTest, ValidatesDeeplyNestedGenerics)
    {
        // Code: func process(data: map<string, vector<vector<scalar>>>) -> scalar {}
        // Tests that TypeAnnotation nodes nest infinitely without dropping context.

        auto ast = parse_code("func process(data: map<string, vector<vector<scalar>>>) -> scalar {}");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);

        ASSERT_EQ(func->parameters.size(), 1);
        auto root_type = func->parameters[0].type.get();

        // Outer type: map
        EXPECT_EQ(root_type->name, "map");
        ASSERT_EQ(root_type->generic_args.size(), 2);

        // First generic arg: string
        EXPECT_EQ(root_type->generic_args[0]->name, "string");

        // Second generic arg: vector
        auto level1_vector = root_type->generic_args[1].get();
        EXPECT_EQ(level1_vector->name, "vector");
        ASSERT_EQ(level1_vector->generic_args.size(), 1);

        // Third generic arg (nested): vector
        auto level2_vector = level1_vector->generic_args[0].get();
        EXPECT_EQ(level2_vector->name, "vector");
        ASSERT_EQ(level2_vector->generic_args.size(), 1);

        // Deepest generic arg: scalar
        auto deepest_scalar = level2_vector->generic_args[0].get();
        EXPECT_EQ(deepest_scalar->name, "scalar");
        EXPECT_TRUE(deepest_scalar->generic_args.empty());
    }

    TEST_F(AstBaseTest, ValidatesBodyStatementsExecutionOrder)
    {
        // Code:
        // func compute() -> scalar {
        //     let a = 10
        //     let b = a * 2
        //     return b
        // }

        auto ast = parse_code("func compute() -> scalar { let a = 10 \n let b = a * 2 \n return b }");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);

        ASSERT_EQ(func->body.size(), 3) << "Expected exactly 3 statements in function body.";

        // Statement 1: let a = 10
        auto stmt_1 = dynamic_cast<Assignment*>(func->body[0].get());
        ASSERT_NE(stmt_1, nullptr);
        ASSERT_EQ(stmt_1->targets.size(), 1);
        EXPECT_EQ(stmt_1->targets[0].first, "a");
        auto val_1 = dynamic_cast<NumberLiteral*>(stmt_1->value.get());
        ASSERT_NE(val_1, nullptr);
        EXPECT_EQ(val_1->value, "10");

        // Statement 2: let b = a * 2
        auto stmt_2 = dynamic_cast<Assignment*>(func->body[1].get());
        ASSERT_NE(stmt_2, nullptr);
        ASSERT_EQ(stmt_2->targets.size(), 1);
        EXPECT_EQ(stmt_2->targets[0].first, "b");
        auto val_2 = dynamic_cast<BinaryExpression*>(stmt_2->value.get());
        ASSERT_NE(val_2, nullptr);
        EXPECT_EQ(val_2->op, TokenType::Star);

        // Statement 3: return b
        auto stmt_3 = dynamic_cast<ReturnStatement*>(func->body[2].get());
        ASSERT_NE(stmt_3, nullptr);
        auto ret_1 = dynamic_cast<IdentifierAccess*>(stmt_3->values[0].get());
        ASSERT_NE(ret_1, nullptr);
        EXPECT_EQ(ret_1->name, "b");
    }

    TEST_F(AstBaseTest, ValidatesTupleReturnStatement)
    {
        // Code: func bounds() -> (scalar, scalar) { return 10, 20 }

        auto ast = parse_code("func bounds() -> (scalar, scalar) { return 10, 20 }");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);

        ASSERT_EQ(func->body.size(), 1);
        auto return_stmt = dynamic_cast<ReturnStatement*>(func->body[0].get());
        ASSERT_NE(return_stmt, nullptr);

        // Validate that the ReturnStatement captured both values natively
        ASSERT_EQ(return_stmt->values.size(), 2);

        auto ret_1 = dynamic_cast<NumberLiteral*>(return_stmt->values[0].get());
        auto ret_2 = dynamic_cast<NumberLiteral*>(return_stmt->values[1].get());

        ASSERT_NE(ret_1, nullptr);
        ASSERT_NE(ret_2, nullptr);
        EXPECT_EQ(ret_1->value, "10");
        EXPECT_EQ(ret_2->value, "20");
    }

    TEST_F(AstBaseTest, ValidatesBodyWithNestedCallsAndSignatureParams)
    {
        // Code:
        // func compute(x: scalar, y: scalar) -> scalar {
        //     let temp = add(x, multiply(y, 2))
        //     return temp
        // }
        // Tests that function parameters (x, y) are correctly parsed as IdentifierAccess
        // inside deeply nested function calls on the right-hand side of an assignment.

        auto ast = parse_code(
            "func compute(x: scalar, y: scalar) -> scalar { let temp = add(a: x, b: multiply(c: y, d: 2)) \n return temp }");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);

        ASSERT_EQ(func->body.size(), 2) << "Expected 2 statements: let and return.";

        // --- Statement 1: let temp = add(...) ---
        auto assign_stmt = dynamic_cast<Assignment*>(func->body[0].get());
        ASSERT_NE(assign_stmt, nullptr);
        EXPECT_EQ(assign_stmt->targets[0].first, "temp");

        // The value is the outer function call: add(...)
        auto add_call = dynamic_cast<FunctionCall*>(assign_stmt->value.get());
        ASSERT_NE(add_call, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(add_call->target.get())->name, "add");
        ASSERT_EQ(add_call->arguments.size(), 2);

        // Argument 1 of add(): x (Original signature param)
        auto arg_x = dynamic_cast<IdentifierAccess*>(add_call->arguments[0].second.get());
        ASSERT_NE(arg_x, nullptr);
        EXPECT_EQ(arg_x->name, "x");

        // Argument 2 of add(): multiply(y, 2)
        auto mult_call = dynamic_cast<FunctionCall*>(add_call->arguments[1].second.get());
        ASSERT_NE(mult_call, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(mult_call->target.get())->name, "multiply");
        ASSERT_EQ(mult_call->arguments.size(), 2);

        // Argument 1 of multiply(): y (Original signature param)
        auto arg_y = dynamic_cast<IdentifierAccess*>(mult_call->arguments[0].second.get());
        ASSERT_NE(arg_y, nullptr);
        EXPECT_EQ(arg_y->name, "y");

        // Argument 2 of multiply(): 2
        auto arg_2 = dynamic_cast<NumberLiteral*>(mult_call->arguments[1].second.get());
        ASSERT_NE(arg_2, nullptr);
        EXPECT_EQ(arg_2->value, "2");
    }

    TEST_F(AstBaseTest, ValidatesConditionalInsideFunctionBody)
    {
        // Code:
        // func max(a: scalar, b: scalar) -> scalar {
        //     let res = if a > b then a else b
        //     return res
        // }
        // Tests that conditional expressions cleanly fit inside block statement assignments.

        auto ast = parse_code(
            "func max(a: scalar, b: scalar) -> scalar { let res = if a > b then a else b \n return res }");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);

        ASSERT_EQ(func->body.size(), 2);

        // --- Statement 1: let res = if ... ---
        auto assign_stmt = dynamic_cast<Assignment*>(func->body[0].get());
        ASSERT_NE(assign_stmt, nullptr);
        EXPECT_EQ(assign_stmt->targets[0].first, "res");

        auto cond_expr = dynamic_cast<ConditionalExpression*>(assign_stmt->value.get());
        ASSERT_NE(cond_expr, nullptr) << "Assigned value must be a ConditionalExpression.";

        // Condition: a > b
        auto condition = dynamic_cast<BinaryExpression*>(cond_expr->condition.get());
        ASSERT_NE(condition, nullptr);
        EXPECT_EQ(condition->op, TokenType::Greater);

        auto cond_left = dynamic_cast<IdentifierAccess*>(condition->left.get());
        auto cond_right = dynamic_cast<IdentifierAccess*>(condition->right.get());
        ASSERT_NE(cond_left, nullptr);
        ASSERT_NE(cond_right, nullptr);
        EXPECT_EQ(cond_left->name, "a");
        EXPECT_EQ(cond_right->name, "b");

        // Then branch: a
        auto then_branch = dynamic_cast<IdentifierAccess*>(cond_expr->then_branch.get());
        ASSERT_NE(then_branch, nullptr);
        EXPECT_EQ(then_branch->name, "a");

        // Else branch: b
        auto else_branch = dynamic_cast<IdentifierAccess*>(cond_expr->else_branch.get());
        ASSERT_NE(else_branch, nullptr);
        EXPECT_EQ(else_branch->name, "b");

        // --- Statement 2: return res ---
        auto return_stmt = dynamic_cast<ReturnStatement*>(func->body[1].get());
        ASSERT_NE(return_stmt, nullptr);
        ASSERT_EQ(return_stmt->values.size(), 1);

        auto ret_val = dynamic_cast<IdentifierAccess*>(return_stmt->values[0].get());
        ASSERT_NE(ret_val, nullptr);
        EXPECT_EQ(ret_val->name, "res");
    }

    TEST_F(AstBaseTest, ValidatesDeeplyNestedGenericsAndTuples)
    {
        // Proves the parser correctly orchestrates:
        // 1. A parameter that is a tuple containing a base type and a generic type.
        // 2. A multiple-return signature (Go-style).
        // 3. The first return is a tuple containing a generic with MULTIPLE generic arguments.
        // 4. The second return is a standalone type.

        auto ast = parse_code(
            "func transform(data: (scalar, Vector<integer>)) -> (scalar, Result<scalar, Error>), Status {}");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);

        // ==========================================
        // PARAMETERS: data: (scalar, Vector<integer>)
        // ==========================================
        ASSERT_EQ(func->parameters.size(), 1) << "Function should have exactly 1 parameter";
        EXPECT_EQ(func->parameters[0].name, "data");

        // The parameter type MUST be a TupleTypeAnnotation
        auto param_tuple = dynamic_cast<TupleTypeAnnotation*>(func->parameters[0].type.get());
        ASSERT_NE(param_tuple, nullptr) << "Parameter type must be a TupleTypeAnnotation";
        ASSERT_EQ(param_tuple->element_types.size(), 2);

        // Element 0: scalar
        EXPECT_EQ(param_tuple->element_types[0]->name, "scalar");
        EXPECT_TRUE(param_tuple->element_types[0]->generic_args.empty());

        // Element 1: Vector<integer>
        EXPECT_EQ(param_tuple->element_types[1]->name, "Vector");
        ASSERT_EQ(param_tuple->element_types[1]->generic_args.size(), 1) << "Vector must have 1 generic argument";
        EXPECT_EQ(param_tuple->element_types[1]->generic_args[0]->name, "integer");

        // ==========================================
        // RETURN TYPES: -> (scalar, Result<scalar, Error>), Status
        // ==========================================
        ASSERT_EQ(func->return_types.size(), 2) << "Function should have exactly 2 disjointed return types";

        // Return 0: (scalar, Result<scalar, Error>)
        auto ret0_tuple = dynamic_cast<TupleTypeAnnotation*>(func->return_types[0].get());
        ASSERT_NE(ret0_tuple, nullptr) << "First return type must be a TupleTypeAnnotation";
        ASSERT_EQ(ret0_tuple->element_types.size(), 2);

        // Return 0, Element 0: scalar
        EXPECT_EQ(ret0_tuple->element_types[0]->name, "scalar");

        // Return 0, Element 1: Result<scalar, Error>
        auto ret0_elem1 = ret0_tuple->element_types[1].get();
        EXPECT_EQ(ret0_elem1->name, "Result");
        ASSERT_EQ(ret0_elem1->generic_args.size(), 2) << "Result must have exactly 2 generic arguments";
        EXPECT_EQ(ret0_elem1->generic_args[0]->name, "scalar");
        EXPECT_EQ(ret0_elem1->generic_args[1]->name, "Error");

        // Return 1: Status
        // Because it is NOT a tuple, it must just be a standard TypeAnnotation
        auto ret1_type = func->return_types[1].get();
        EXPECT_EQ(ret1_type->name, "Status");
        EXPECT_TRUE(ret1_type->generic_args.empty());

        // Verify it is definitively NOT a tuple annotation masquerading as a base annotation
        EXPECT_EQ(dynamic_cast<TupleTypeAnnotation*>(ret1_type), nullptr);
    }

    TEST_F(AstBaseTest, ValidatesMultipleGenericArgumentsInSignature)
    {
        // Proves the parser correctly identifies and nests multiple comma-separated
        // generic arguments for both function parameters and return types.

        auto ast = parse_code("func test(a: Input<A, B>) -> Result<T, E> {}");
        auto func = get_first_func(ast);

        ASSERT_NE(func, nullptr) << "Execution step must be a FunctionDefinition";
        EXPECT_EQ(func->name, "test");

        // ==========================================
        // PARAMETERS: a: Input<A, B>
        // ==========================================
        ASSERT_EQ(func->parameters.size(), 1) << "Function should have exactly 1 parameter";
        EXPECT_EQ(func->parameters[0].name, "a");

        auto param_type = func->parameters[0].type.get();
        ASSERT_NE(param_type, nullptr);
        EXPECT_EQ(param_type->name, "Input");

        // Validate the generic arguments <A, B>
        ASSERT_EQ(param_type->generic_args.size(), 2) << "Input type must have exactly 2 generic arguments";

        // Generic Arg 0: A
        auto param_generic_0 = param_type->generic_args[0].get();
        EXPECT_EQ(param_generic_0->name, "A");
        EXPECT_TRUE(param_generic_0->generic_args.empty()) << "Generic arg A should not have its own nested generics";

        // Generic Arg 1: B
        auto param_generic_1 = param_type->generic_args[1].get();
        EXPECT_EQ(param_generic_1->name, "B");
        EXPECT_TRUE(param_generic_1->generic_args.empty()) << "Generic arg B should not have its own nested generics";

        // ==========================================
        // RETURN TYPES: -> Result<T, E>
        // ==========================================
        ASSERT_EQ(func->return_types.size(), 1) << "Function should have exactly 1 return type";

        auto return_type = func->return_types[0].get();
        ASSERT_NE(return_type, nullptr);
        EXPECT_EQ(return_type->name, "Result");

        // Validate the generic arguments <T, E>
        ASSERT_EQ(return_type->generic_args.size(), 2) << "Result type must have exactly 2 generic arguments";

        // Generic Arg 0: T
        auto return_generic_0 = return_type->generic_args[0].get();
        EXPECT_EQ(return_generic_0->name, "T");
        EXPECT_TRUE(return_generic_0->generic_args.empty()) << "Generic arg T should not have its own nested generics";

        // Generic Arg 1: E
        auto return_generic_1 = return_type->generic_args[1].get();
        EXPECT_EQ(return_generic_1->name, "E");
        EXPECT_TRUE(return_generic_1->generic_args.empty()) << "Generic arg E should not have its own nested generics";
    }

    TEST_F(AstBaseTest, ValidatesMultipleReturnFunction)
    {
        // Proves the parser correctly identifies and parses multiple value returning functions

        auto ast = parse_code("func test(a: Input<A, B>) -> Result<T, E>, scalar {}");
        auto func = get_first_func(ast);

        ASSERT_NE(func, nullptr) << "Execution step must be a FunctionDefinition";
        EXPECT_EQ(func->name, "test");

        // ==========================================
        // PARAMETERS: a: Input<A, B>
        // ==========================================
        ASSERT_EQ(func->parameters.size(), 1) << "Function should have exactly 1 parameter";
        EXPECT_EQ(func->parameters[0].name, "a");

        auto param_type = func->parameters[0].type.get();
        ASSERT_NE(param_type, nullptr);
        EXPECT_EQ(param_type->name, "Input");

        // Validate the generic arguments <A, B>
        ASSERT_EQ(param_type->generic_args.size(), 2) << "Input type must have exactly 2 generic arguments";

        // Generic Arg 0: A
        auto param_generic_0 = param_type->generic_args[0].get();
        EXPECT_EQ(param_generic_0->name, "A");
        EXPECT_TRUE(param_generic_0->generic_args.empty()) << "Generic arg A should not have its own nested generics";

        // Generic Arg 1: B
        auto param_generic_1 = param_type->generic_args[1].get();
        EXPECT_EQ(param_generic_1->name, "B");
        EXPECT_TRUE(param_generic_1->generic_args.empty()) << "Generic arg B should not have its own nested generics";

        // ==========================================
        // RETURN TYPES: -> Result<T, E>
        // ==========================================
        ASSERT_EQ(func->return_types.size(), 2) << "Function should have exactly 2 return types";

        auto return_type_1 = func->return_types[0].get();
        ASSERT_NE(return_type_1, nullptr);
        EXPECT_EQ(return_type_1->name, "Result");

        // Validate the generic arguments <T, E>
        ASSERT_EQ(return_type_1->generic_args.size(), 2) << "Result type must have exactly 2 generic arguments";

        // Generic Arg 0: T
        auto return_generic_0 = return_type_1->generic_args[0].get();
        EXPECT_EQ(return_generic_0->name, "T");
        EXPECT_TRUE(return_generic_0->generic_args.empty()) << "Generic arg T should not have its own nested generics";

        // Generic Arg 1: E
        auto return_generic_1 = return_type_1->generic_args[1].get();
        EXPECT_EQ(return_generic_1->name, "E");
        EXPECT_TRUE(return_generic_1->generic_args.empty()) << "Generic arg E should not have its own nested generics";

        // Scalar return type
        auto return_type_2 = func->return_types[1].get();
        ASSERT_NE(return_type_2, nullptr);
        EXPECT_EQ(return_type_2->name, "scalar");
        EXPECT_EQ(return_type_2->generic_args.size(), 0);
    }

    TEST_F(AstBaseTest, ValidatesFunctionWithSimpleDefaultParameters)
    {
        auto ast = parse_code(R"(func config(a: int = 1, b: string = "hello", c: boolean = true) -> void {})");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);
        ASSERT_EQ(func->parameters.size(), 3);

        // a: int = 1
        EXPECT_EQ(func->parameters[0].name, "a");
        auto def_a = dynamic_cast<NumberLiteral*>(func->parameters[0].default_value.get());
        ASSERT_NE(def_a, nullptr);
        EXPECT_EQ(def_a->value, "1");

        // b: string = "hello"
        EXPECT_EQ(func->parameters[1].name, "b");
        auto def_b = dynamic_cast<StringLiteral*>(func->parameters[1].default_value.get());
        ASSERT_NE(def_b, nullptr);
        EXPECT_EQ(def_b->value, "\"hello\"");

        // c: boolean = true
        EXPECT_EQ(func->parameters[2].name, "c");
        auto def_c = dynamic_cast<BooleanLiteral*>(func->parameters[2].default_value.get());
        ASSERT_NE(def_c, nullptr);
        EXPECT_EQ(def_c->value, true);
    }

    TEST_F(AstBaseTest, ValidatesFunctionWithCollectionDefaultParameters)
    {
        auto ast = parse_code(
            R"(func setup(vec: vector = [1, 2], map: dict = {x: 10}, tup: tuple = (true, false)) -> void {})");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);
        ASSERT_EQ(func->parameters.size(), 3);

        // vec: vector = [1, 2]
        EXPECT_EQ(func->parameters[0].name, "vec");
        auto def_vec = dynamic_cast<TensorLiteral*>(func->parameters[0].default_value.get());
        ASSERT_NE(def_vec, nullptr);
        ASSERT_EQ(def_vec->elements.size(), 2);

        auto vec_el_1 = dynamic_cast<NumberLiteral*>(def_vec->elements[0].get());
        auto vec_el_2 = dynamic_cast<NumberLiteral*>(def_vec->elements[1].get());
        ASSERT_NE(vec_el_1, nullptr);
        ASSERT_NE(vec_el_2, nullptr);
        EXPECT_EQ(vec_el_1->value, "1");
        EXPECT_EQ(vec_el_2->value, "2");

        // map: dict = {"x": 10}
        EXPECT_EQ(func->parameters[1].name, "map");
        auto def_map = dynamic_cast<DictLiteral*>(func->parameters[1].default_value.get());
        ASSERT_NE(def_map, nullptr);
        ASSERT_EQ(def_map->elements.size(), 1);
        EXPECT_EQ(def_map->elements[0].key, "x");

        auto map_val = dynamic_cast<NumberLiteral*>(def_map->elements[0].value.get());
        ASSERT_NE(map_val, nullptr);
        EXPECT_EQ(map_val->value, "10");

        // tup: tuple = (true, false)
        EXPECT_EQ(func->parameters[2].name, "tup");
        auto def_tup = dynamic_cast<TupleLiteral*>(func->parameters[2].default_value.get());
        ASSERT_NE(def_tup, nullptr);
        ASSERT_EQ(def_tup->elements.size(), 2);

        auto tup_el_1 = dynamic_cast<BooleanLiteral*>(def_tup->elements[0].get());
        auto tup_el_2 = dynamic_cast<BooleanLiteral*>(def_tup->elements[1].get());
        ASSERT_NE(tup_el_1, nullptr);
        ASSERT_NE(tup_el_2, nullptr);
        EXPECT_EQ(tup_el_1->value, true);
        EXPECT_EQ(tup_el_2->value, false);
    }

    TEST_F(AstBaseTest, ValidatesFunctionWithComplexDefaultParameters)
    {
        auto ast = parse_code(
            R"(func process(mode: Mode = System.Auto, data: vector = [{val: Color.Red}]) -> void {})");
        auto func = get_first_func(ast);
        ASSERT_NE(func, nullptr);
        ASSERT_EQ(func->parameters.size(), 2);

        // mode: Mode = System.Auto
        EXPECT_EQ(func->parameters[0].name, "mode");
        auto def_mode = dynamic_cast<DotAccess*>(func->parameters[0].default_value.get());
        ASSERT_NE(def_mode, nullptr);

        auto target_sys = dynamic_cast<IdentifierAccess*>(def_mode->target.get());
        ASSERT_NE(target_sys, nullptr);
        EXPECT_EQ(target_sys->name, "System");
        EXPECT_EQ(def_mode->property_name, "Auto");

        // data: vector = [{"val": Color.Red}]
        EXPECT_EQ(func->parameters[1].name, "data");
        auto def_data = dynamic_cast<TensorLiteral*>(func->parameters[1].default_value.get());
        ASSERT_NE(def_data, nullptr);
        ASSERT_EQ(def_data->elements.size(), 1);

        auto dict_elem = dynamic_cast<DictLiteral*>(def_data->elements[0].get());
        ASSERT_NE(dict_elem, nullptr);
        ASSERT_EQ(dict_elem->elements.size(), 1);
        EXPECT_EQ(dict_elem->elements[0].key, "val");

        auto nested_dot = dynamic_cast<DotAccess*>(dict_elem->elements[0].value.get());
        ASSERT_NE(nested_dot, nullptr);

        auto target_color = dynamic_cast<IdentifierAccess*>(nested_dot->target.get());
        ASSERT_NE(target_color, nullptr);
        EXPECT_EQ(target_color->name, "Color");
        EXPECT_EQ(nested_dot->property_name, "Red");
    }

    TEST_F(AstBaseTest, ValidatesSelfInFunctionDefaultParameters)
    {
        // Proves that 'self' is structurally valid as a default value for a parameter.

        auto ast = parse_code("func apply(rate: float = self.default_rate) -> void {}");
        ASSERT_EQ(ast->function_definitions.size(), 1);

        auto& params = ast->function_definitions[0]->parameters;
        ASSERT_EQ(params.size(), 1);
        EXPECT_EQ(params[0].name, "rate");

        auto default_val = dynamic_cast<DotAccess*>(params[0].default_value.get());
        ASSERT_NE(default_val, nullptr);
        EXPECT_EQ(default_val->property_name, "default_rate");
        ASSERT_NE(dynamic_cast<SelfExpression*>(default_val->target.get()), nullptr);
    }

    TEST_F(AstBaseTest, ValidatesSelfInReturnAndCollections)
    {
        // Proves 'self' can be used in return statements and inside Tensor/List literals.

        std::string code =
            "func get_data() -> int {\n"
            "  let list = [self.a, self.b]\n"
            "  return self.status, self.value\n"
            "}";

        auto ast = parse_code(code);
        ASSERT_EQ(ast->function_definitions.size(), 1);
        auto& body = ast->function_definitions[0]->body;
        ASSERT_EQ(body.size(), 2);

        // Check Assignment: [self.a, self.b]
        auto assign = dynamic_cast<Assignment*>(body[0].get());
        auto tensor = dynamic_cast<TensorLiteral*>(assign->value.get());
        ASSERT_NE(tensor, nullptr);
        ASSERT_EQ(tensor->elements.size(), 2);
        ASSERT_NE(dynamic_cast<SelfExpression*>(dynamic_cast<DotAccess*>(tensor->elements[0].get())->target.get()),
                  nullptr);

        // Check Return: self.status, self.value
        auto ret = dynamic_cast<ReturnStatement*>(body[1].get());
        ASSERT_NE(ret, nullptr);
        ASSERT_EQ(ret->values.size(), 2);
        ASSERT_NE(dynamic_cast<SelfExpression*>(dynamic_cast<DotAccess*>(ret->values[0].get())->target.get()), nullptr);
        ASSERT_NE(dynamic_cast<SelfExpression*>(dynamic_cast<DotAccess*>(ret->values[1].get())->target.get()), nullptr);
    }

    TEST_F(AstBaseTest, ValidatesFunctionLevelAndParameterLevelModifiersSeparately)
    {
        // Ensures that the parser doesn't accidentally attach function modifiers to the first parameter
        auto ast = parse_code(
            "@export @api(route: \"/test\") "
            "func execute(@mut state: int, @optional config: Dict) -> void {}"
        );

        ASSERT_EQ(ast->function_definitions.size(), 1);
        auto func_node = ast->function_definitions[0].get();

        // Check Function Modifiers
        ASSERT_EQ(func_node->modifiers.size(), 2);
        EXPECT_EQ(func_node->modifiers[0].name, "export");
        EXPECT_EQ(func_node->modifiers[1].name, "api");

        // Check Parameter Modifiers
        ASSERT_EQ(func_node->parameters.size(), 2);

        // Param 0: @mut state
        EXPECT_EQ(func_node->parameters[0].name, "state");
        ASSERT_EQ(func_node->parameters[0].modifiers.size(), 1);
        EXPECT_EQ(func_node->parameters[0].modifiers[0].name, "mut");

        // Param 1: @optional config
        EXPECT_EQ(func_node->parameters[1].name, "config");
        ASSERT_EQ(func_node->parameters[1].modifiers.size(), 1);
        EXPECT_EQ(func_node->parameters[1].modifiers[0].name, "optional");
    }

    TEST_F(AstBaseTest, ValidatesModifierOnFunctionParameterWithDefaultValue)
    {
        // Ensure modifiers don't break default parameter value parsing
        auto ast = parse_code("func connect(@timeout(ms: 500) duration: int = 1000) -> void {}");

        ASSERT_EQ(ast->function_definitions.size(), 1);
        auto func_node = ast->function_definitions[0].get();

        ASSERT_EQ(func_node->parameters.size(), 1);
        const auto& param = func_node->parameters[0];

        EXPECT_EQ(param.name, "duration");

        // Verify Modifier
        ASSERT_EQ(param.modifiers.size(), 1);
        EXPECT_EQ(param.modifiers[0].name, "timeout");

        // Verify Default Value
        ASSERT_NE(param.default_value, nullptr);
        auto default_val = dynamic_cast<NumberLiteral*>(param.default_value.get());
        ASSERT_NE(default_val, nullptr);
        EXPECT_EQ(default_val->value, "1000");
    }
}
