#include "../ast_base_test.h"
using namespace valuascript::compiler::test;

TEST_F(AstBaseTest, ValidatesSimpleModifierOnAssignment) {
    auto ast = parse_code("@export let config = 100");
    
    ASSERT_EQ(ast->execution_steps.size(), 1);
    auto assign_node = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(assign_node, nullptr);

    // Check Modifiers
    ASSERT_EQ(assign_node->modifiers.size(), 1);
    const auto& mod = assign_node->modifiers[0];
    EXPECT_EQ(mod.name, "export");
    EXPECT_TRUE(mod.arguments.empty());
}

TEST_F(AstBaseTest, ValidatesModifierWithEmptyParentheses) {
    auto ast = parse_code("@inject() var service = get_service()");
    
    ASSERT_EQ(ast->execution_steps.size(), 1);
    auto assign_node = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    ASSERT_NE(assign_node, nullptr);

    ASSERT_EQ(assign_node->modifiers.size(), 1);
    const auto& mod = assign_node->modifiers[0];
    EXPECT_EQ(mod.name, "inject");
    
    // Explicitly empty parens should yield zero arguments
    EXPECT_TRUE(mod.arguments.empty());
}

TEST_F(AstBaseTest, ValidatesModifierWithPrimitiveArgumentsOnFunction) {
    auto ast = parse_code(R"(@api(route: "/users", method: "GET", timeout: 30) func get_users() -> void {})");
    
    ASSERT_EQ(ast->function_definitions.size(), 1);
    auto func_node = ast->function_definitions[0].get();

    ASSERT_EQ(func_node->modifiers.size(), 1);
    const auto& mod = func_node->modifiers[0];
    
    EXPECT_EQ(mod.name, "api");
    ASSERT_EQ(mod.arguments.size(), 3);

    // Arg 0: route
    EXPECT_EQ(mod.arguments[0].first, "route");
    auto arg0_val = dynamic_cast<StringLiteral*>(mod.arguments[0].second.get());
    ASSERT_NE(arg0_val, nullptr);
    EXPECT_EQ(arg0_val->value, "\"/users\"");

    // Arg 1: method
    EXPECT_EQ(mod.arguments[1].first, "method");
    auto arg1_val = dynamic_cast<StringLiteral*>(mod.arguments[1].second.get());
    ASSERT_NE(arg1_val, nullptr);
    EXPECT_EQ(arg1_val->value, "\"GET\"");

    // Arg 2: timeout
    EXPECT_EQ(mod.arguments[2].first, "timeout");
    auto arg2_val = dynamic_cast<NumberLiteral*>(mod.arguments[2].second.get());
    ASSERT_NE(arg2_val, nullptr);
    EXPECT_EQ(arg2_val->value, "30");
}

TEST_F(AstBaseTest, ValidatesStackedModifiersOnStruct) {
    auto ast = parse_code("@serializable @packed(align: 4) struct Data { id: int }");
    
    ASSERT_EQ(ast->struct_definitions.size(), 1);
    auto struct_node = ast->struct_definitions[0].get();

    // Check multiple modifiers attached to the same node
    ASSERT_EQ(struct_node->modifiers.size(), 2);

    // Mod 0: @serializable
    EXPECT_EQ(struct_node->modifiers[0].name, "serializable");
    EXPECT_TRUE(struct_node->modifiers[0].arguments.empty());

    // Mod 1: @packed(align: 4)
    EXPECT_EQ(struct_node->modifiers[1].name, "packed");
    ASSERT_EQ(struct_node->modifiers[1].arguments.size(), 1);
    
    EXPECT_EQ(struct_node->modifiers[1].arguments[0].first, "align");
    auto align_val = dynamic_cast<NumberLiteral*>(struct_node->modifiers[1].arguments[0].second.get());
    ASSERT_NE(align_val, nullptr);
    EXPECT_EQ(align_val->value, "4");
}

TEST_F(AstBaseTest, ValidatesModifierWithComplexExpressionsOnEnum) {
    // Proves that modifier arguments recursively use the full expression parser
    auto ast = parse_code("@config(timeout: 60 * 2, fallback: get_default()) enum State: int { A = 1 }");
    
    ASSERT_EQ(ast->enum_definitions.size(), 1);
    auto enum_node = ast->enum_definitions[0].get();

    ASSERT_EQ(enum_node->modifiers.size(), 1);
    const auto& mod = enum_node->modifiers[0];
    
    EXPECT_EQ(mod.name, "config");
    ASSERT_EQ(mod.arguments.size(), 2);

    // Arg 0: timeout (Math expression)
    EXPECT_EQ(mod.arguments[0].first, "timeout");
    auto math_expr = dynamic_cast<BinaryExpression*>(mod.arguments[0].second.get());
    ASSERT_NE(math_expr, nullptr) << "timeout argument must be parsed as a BinaryExpression";
    EXPECT_EQ(math_expr->op, TokenType::Star);
    
    auto left_math = dynamic_cast<NumberLiteral*>(math_expr->left.get());
    ASSERT_NE(left_math, nullptr);
    EXPECT_EQ(left_math->value, "60");

    // Arg 1: fallback (Function call)
    EXPECT_EQ(mod.arguments[1].first, "fallback");
    auto call_expr = dynamic_cast<FunctionCall*>(mod.arguments[1].second.get());
    ASSERT_NE(call_expr, nullptr) << "fallback argument must be parsed as a FunctionCall";
    
    auto call_target = dynamic_cast<IdentifierAccess*>(call_expr->target.get());
    ASSERT_NE(call_target, nullptr);
    EXPECT_EQ(call_target->name, "get_default");
    EXPECT_TRUE(call_expr->arguments.empty());
}

TEST_F(AstBaseTest, ValidatesModifierWithTensorLiteral) {
    auto ast = parse_code("@matrix(shape: [2, 3]) let m = 0");
    
    auto assign_node = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    const auto& mod = assign_node->modifiers[0];
    
    EXPECT_EQ(mod.name, "matrix");
    ASSERT_EQ(mod.arguments.size(), 1);
    
    EXPECT_EQ(mod.arguments[0].first, "shape");
    auto tensor_val = dynamic_cast<TensorLiteral*>(mod.arguments[0].second.get());
    ASSERT_NE(tensor_val, nullptr);
    ASSERT_EQ(tensor_val->elements.size(), 2);
    
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(tensor_val->elements[0].get())->value, "2");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(tensor_val->elements[1].get())->value, "3");
}

TEST_F(AstBaseTest, ValidatesNestedDictLiteralInModifier) {
    auto ast = parse_code("@meta(config: { retries: 3, flags: { active: true, safe: false } }) let x = 1");
    auto assign_node = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    const auto& mod = assign_node->modifiers[0];

    EXPECT_EQ(mod.arguments[0].first, "config");
    auto outer_dict = dynamic_cast<DictLiteral*>(mod.arguments[0].second.get());
    ASSERT_NE(outer_dict, nullptr);
    ASSERT_EQ(outer_dict->pairs.size(), 2);

    // Pair 0: retries: 3
    EXPECT_EQ(outer_dict->pairs[0].first, "retries");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(outer_dict->pairs[0].second.get())->value, "3");

    // Pair 1: flags: { ... }
    EXPECT_EQ(outer_dict->pairs[1].first, "flags");
    auto inner_dict = dynamic_cast<DictLiteral*>(outer_dict->pairs[1].second.get());
    ASSERT_NE(inner_dict, nullptr);
    ASSERT_EQ(inner_dict->pairs.size(), 2);

    EXPECT_EQ(inner_dict->pairs[0].first, "active");
    EXPECT_EQ(dynamic_cast<BooleanLiteral*>(inner_dict->pairs[0].second.get())->value, true);

    EXPECT_EQ(inner_dict->pairs[1].first, "safe");
    EXPECT_EQ(dynamic_cast<BooleanLiteral*>(inner_dict->pairs[1].second.get())->value, false);
}

TEST_F(AstBaseTest, ValidatesTupleLiteralInModifier) {
    auto ast = parse_code("@position(coords: (10.5, -20.0, 5)) let obj = 0");
    auto assign_node = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    const auto& mod = assign_node->modifiers[0];

    EXPECT_EQ(mod.arguments[0].first, "coords");
    auto tuple_lit = dynamic_cast<TupleLiteral*>(mod.arguments[0].second.get());
    ASSERT_NE(tuple_lit, nullptr);
    ASSERT_EQ(tuple_lit->elements.size(), 3);

    EXPECT_EQ(dynamic_cast<NumberLiteral*>(tuple_lit->elements[0].get())->value, "10.5");

    auto unary_minus = dynamic_cast<UnaryExpression*>(tuple_lit->elements[1].get());
    ASSERT_NE(unary_minus, nullptr);
    EXPECT_EQ(unary_minus->op, TokenType::Minus);
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(unary_minus->right.get())->value, "20.0");

    EXPECT_EQ(dynamic_cast<NumberLiteral*>(tuple_lit->elements[2].get())->value, "5");
}

TEST_F(AstBaseTest, ValidatesNestedTensorLiteralInModifier) {
    auto ast = parse_code("@kernel(weights: [[1, 0], [0, 1]]) func process() -> void {}");
    auto func_node = ast->function_definitions[0].get();
    const auto& mod = func_node->modifiers[0];

    EXPECT_EQ(mod.arguments[0].first, "weights");
    auto outer_tensor = dynamic_cast<TensorLiteral*>(mod.arguments[0].second.get());
    ASSERT_NE(outer_tensor, nullptr);
    ASSERT_EQ(outer_tensor->elements.size(), 2);

    // Inner Tensor 1: [1, 0]
    auto inner_tensor_1 = dynamic_cast<TensorLiteral*>(outer_tensor->elements[0].get());
    ASSERT_NE(inner_tensor_1, nullptr);
    ASSERT_EQ(inner_tensor_1->elements.size(), 2);
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner_tensor_1->elements[0].get())->value, "1");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner_tensor_1->elements[1].get())->value, "0");

    // Inner Tensor 2: [0, 1]
    auto inner_tensor_2 = dynamic_cast<TensorLiteral*>(outer_tensor->elements[1].get());
    ASSERT_NE(inner_tensor_2, nullptr);
    ASSERT_EQ(inner_tensor_2->elements.size(), 2);
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner_tensor_2->elements[0].get())->value, "0");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner_tensor_2->elements[1].get())->value, "1");
}

TEST_F(AstBaseTest, ValidatesDeepDotAccessInModifier) {
    auto ast = parse_code("@ui(theme: App.Theme.Dark, icon: Icons.Warning) struct Window {}");
    auto struct_node = ast->struct_definitions[0].get();
    const auto& mod = struct_node->modifiers[0];

    ASSERT_EQ(mod.arguments.size(), 2);

    // Arg 0: App.Theme.Dark
    EXPECT_EQ(mod.arguments[0].first, "theme");
    auto dot1 = dynamic_cast<DotAccess*>(mod.arguments[0].second.get());
    ASSERT_NE(dot1, nullptr);
    EXPECT_EQ(dot1->property_name, "Dark");

    auto dot2 = dynamic_cast<DotAccess*>(dot1->target.get());
    ASSERT_NE(dot2, nullptr);
    EXPECT_EQ(dot2->property_name, "Theme");

    auto id = dynamic_cast<IdentifierAccess*>(dot2->target.get());
    ASSERT_NE(id, nullptr);
    EXPECT_EQ(id->name, "App");

    // Arg 1: Icons.Warning
    EXPECT_EQ(mod.arguments[1].first, "icon");
    auto icon_dot = dynamic_cast<DotAccess*>(mod.arguments[1].second.get());
    ASSERT_NE(icon_dot, nullptr);
    EXPECT_EQ(icon_dot->property_name, "Warning");
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(icon_dot->target.get())->name, "Icons");
}

TEST_F(AstBaseTest, ValidatesBracketAccessAndSlicingInModifier) {
    auto ast = parse_code("@memory(chunk: buffer[0:256], offset: limits[5]) let block = 0");
    auto assign_node = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    const auto& mod = assign_node->modifiers[0];

    ASSERT_EQ(mod.arguments.size(), 2);

    // Arg 0: buffer[0:256] (Slicing)
    EXPECT_EQ(mod.arguments[0].first, "chunk");
    auto slice_access = dynamic_cast<BracketAccess*>(mod.arguments[0].second.get());
    ASSERT_NE(slice_access, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(slice_access->target.get())->name, "buffer");

    auto slice_bin = dynamic_cast<BinaryExpression*>(slice_access->index.get());
    ASSERT_NE(slice_bin, nullptr);
    EXPECT_EQ(slice_bin->op, TokenType::Colon);
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(slice_bin->left.get())->value, "0");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(slice_bin->right.get())->value, "256");

    // Arg 1: limits[5] (Single Index)
    EXPECT_EQ(mod.arguments[1].first, "offset");
    auto index_access = dynamic_cast<BracketAccess*>(mod.arguments[1].second.get());
    ASSERT_NE(index_access, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(index_access->target.get())->name, "limits");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(index_access->index.get())->value, "5");
}

TEST_F(AstBaseTest, ValidatesFunctionCallWithArgumentsInModifier) {
    auto ast = parse_code("@retry(strategy: exponential_backoff(base: 2, max: 10)) func fetch() -> void {}");
    auto func_node = ast->function_definitions[0].get();
    const auto& mod = func_node->modifiers[0];

    EXPECT_EQ(mod.arguments[0].first, "strategy");
    auto func_call = dynamic_cast<FunctionCall*>(mod.arguments[0].second.get());
    ASSERT_NE(func_call, nullptr);

    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(func_call->target.get())->name, "exponential_backoff");
    ASSERT_EQ(func_call->arguments.size(), 2);

    EXPECT_EQ(func_call->arguments[0].first, "base");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(func_call->arguments[0].second.get())->value, "2");

    EXPECT_EQ(func_call->arguments[1].first, "max");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(func_call->arguments[1].second.get())->value, "10");
}

TEST_F(AstBaseTest, ValidatesOmnibusModifierStressTest) {
    // Tests a modifier wrapping a dict, containing a function call, containing a tuple and dot/bracket access
    std::string code =
        "@complex(payload: {"
        "    data: fetch(args: (State.Active, buffer[1])),"
        "    fallback: [ [1], [2] ]"
        "}) let y = 0";

    auto ast = parse_code(code);
    auto assign_node = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
    const auto& mod = assign_node->modifiers[0];

    // payload: { ... }
    EXPECT_EQ(mod.arguments[0].first, "payload");
    auto payload_dict = dynamic_cast<DictLiteral*>(mod.arguments[0].second.get());
    ASSERT_NE(payload_dict, nullptr);
    ASSERT_EQ(payload_dict->pairs.size(), 2);

    // data: fetch(args: (...))
    EXPECT_EQ(payload_dict->pairs[0].first, "data");
    auto fetch_call = dynamic_cast<FunctionCall*>(payload_dict->pairs[0].second.get());
    ASSERT_NE(fetch_call, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(fetch_call->target.get())->name, "fetch");
    ASSERT_EQ(fetch_call->arguments.size(), 1);

    // args: (State.Active, buffer[1])
    EXPECT_EQ(fetch_call->arguments[0].first, "args");
    auto args_tuple = dynamic_cast<TupleLiteral*>(fetch_call->arguments[0].second.get());
    ASSERT_NE(args_tuple, nullptr);
    ASSERT_EQ(args_tuple->elements.size(), 2);

    // Tuple element 0: State.Active
    auto state_dot = dynamic_cast<DotAccess*>(args_tuple->elements[0].get());
    ASSERT_NE(state_dot, nullptr);
    EXPECT_EQ(state_dot->property_name, "Active");
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(state_dot->target.get())->name, "State");

    // Tuple element 1: buffer[1]
    auto buffer_bracket = dynamic_cast<BracketAccess*>(args_tuple->elements[1].get());
    ASSERT_NE(buffer_bracket, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(buffer_bracket->target.get())->name, "buffer");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(buffer_bracket->index.get())->value, "1");

    // fallback: [ [1], [2] ]
    EXPECT_EQ(payload_dict->pairs[1].first, "fallback");
    auto fallback_tensor = dynamic_cast<TensorLiteral*>(payload_dict->pairs[1].second.get());
    ASSERT_NE(fallback_tensor, nullptr);
    ASSERT_EQ(fallback_tensor->elements.size(), 2);
}