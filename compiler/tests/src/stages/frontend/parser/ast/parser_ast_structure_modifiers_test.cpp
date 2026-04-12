#include "frontend/parser/ast_base_test.h"

namespace valuascript::compiler::test {
    TEST_F(AstBaseTest, ValidatesSimpleModifierOnAssignment) {
        auto ast = parse_code("@export let config = 100");

        ASSERT_EQ(ast->execution_steps.size(), 1);
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assign_node, nullptr);

        // Check Modifiers
        ASSERT_EQ(assign_node->modifiers.size(), 1);
        const auto &mod = assign_node->modifiers[0];
        EXPECT_EQ(mod.name, "export");
        EXPECT_TRUE(mod.arguments.empty());
    }

    TEST_F(AstBaseTest, ValidatesModifierWithEmptyParentheses) {
        auto ast = parse_code("@inject() var service = get_service()");

        ASSERT_EQ(ast->execution_steps.size(), 1);
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assign_node, nullptr);

        ASSERT_EQ(assign_node->modifiers.size(), 1);
        const auto &mod = assign_node->modifiers[0];
        EXPECT_EQ(mod.name, "inject");

        // Explicitly empty parens should yield zero arguments
        EXPECT_TRUE(mod.arguments.empty());
    }

    TEST_F(AstBaseTest, ValidatesModifierWithPrimitiveArgumentsOnFunction) {
        auto ast = parse_code(R"(@api(route: "/users", method: "GET", timeout: 30) func get_users() -> void {})");

        ASSERT_EQ(ast->function_definitions.size(), 1);
        auto func_node = ast->function_definitions[0].get();

        ASSERT_EQ(func_node->modifiers.size(), 1);
        const auto &mod = func_node->modifiers[0];

        EXPECT_EQ(mod.name, "api");
        ASSERT_EQ(mod.arguments.size(), 3);

        // Arg 0: route
        EXPECT_EQ(mod.arguments[0].first, "route");
        auto arg0_val = dynamic_cast<StringLiteral *>(mod.arguments[0].second.get());
        ASSERT_NE(arg0_val, nullptr);
        EXPECT_EQ(arg0_val->value, "\"/users\"");

        // Arg 1: method
        EXPECT_EQ(mod.arguments[1].first, "method");
        auto arg1_val = dynamic_cast<StringLiteral *>(mod.arguments[1].second.get());
        ASSERT_NE(arg1_val, nullptr);
        EXPECT_EQ(arg1_val->value, "\"GET\"");

        // Arg 2: timeout
        EXPECT_EQ(mod.arguments[2].first, "timeout");
        auto arg2_val = dynamic_cast<NumberLiteral *>(mod.arguments[2].second.get());
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
        auto align_val = dynamic_cast<NumberLiteral *>(struct_node->modifiers[1].arguments[0].second.get());
        ASSERT_NE(align_val, nullptr);
        EXPECT_EQ(align_val->value, "4");
    }

    TEST_F(AstBaseTest, ValidatesModifierWithComplexExpressionsOnEnum) {
        // Proves that modifier arguments recursively use the full expression parser
        auto ast = parse_code("@config(timeout: 60 * 2, fallback: get_default()) enum State: int { A = 1 }");

        ASSERT_EQ(ast->enum_definitions.size(), 1);
        auto enum_node = ast->enum_definitions[0].get();

        ASSERT_EQ(enum_node->modifiers.size(), 1);
        const auto &mod = enum_node->modifiers[0];

        EXPECT_EQ(mod.name, "config");
        ASSERT_EQ(mod.arguments.size(), 2);

        // Arg 0: timeout (Math expression)
        EXPECT_EQ(mod.arguments[0].first, "timeout");
        auto math_expr = dynamic_cast<BinaryExpression *>(mod.arguments[0].second.get());
        ASSERT_NE(math_expr, nullptr) << "timeout argument must be parsed as a BinaryExpression";
        EXPECT_EQ(math_expr->op, TokenType::Star);

        auto left_math = dynamic_cast<NumberLiteral *>(math_expr->left.get());
        ASSERT_NE(left_math, nullptr);
        EXPECT_EQ(left_math->value, "60");

        // Arg 1: fallback (Function call)
        EXPECT_EQ(mod.arguments[1].first, "fallback");
        auto call_expr = dynamic_cast<FunctionCall *>(mod.arguments[1].second.get());
        ASSERT_NE(call_expr, nullptr) << "fallback argument must be parsed as a FunctionCall";

        auto call_target = dynamic_cast<IdentifierAccess *>(call_expr->target.get());
        ASSERT_NE(call_target, nullptr);
        EXPECT_EQ(call_target->name, "get_default");
        EXPECT_TRUE(call_expr->arguments.empty());
    }

    TEST_F(AstBaseTest, ValidatesModifierWithTensorLiteral) {
        auto ast = parse_code("@matrix(shape: [2, 3]) let m = 0");

        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        const auto &mod = assign_node->modifiers[0];

        EXPECT_EQ(mod.name, "matrix");
        ASSERT_EQ(mod.arguments.size(), 1);

        EXPECT_EQ(mod.arguments[0].first, "shape");
        auto tensor_val = dynamic_cast<TensorLiteral *>(mod.arguments[0].second.get());
        ASSERT_NE(tensor_val, nullptr);
        ASSERT_EQ(tensor_val->elements.size(), 2);

        EXPECT_EQ(dynamic_cast<NumberLiteral*>(tensor_val->elements[0].get())->value, "2");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(tensor_val->elements[1].get())->value, "3");
    }

    TEST_F(AstBaseTest, ValidatesNestedDictLiteralInModifier) {
        auto ast = parse_code("@meta(config: { retries: 3, flags: { active: true, safe: false } }) let x = 1");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        const auto &mod = assign_node->modifiers[0];

        EXPECT_EQ(mod.arguments[0].first, "config");
        auto outer_dict = dynamic_cast<DictLiteral *>(mod.arguments[0].second.get());
        ASSERT_NE(outer_dict, nullptr);
        ASSERT_EQ(outer_dict->elements.size(), 2);

        // Pair 0: retries: 3
        EXPECT_EQ(outer_dict->elements[0].key, "retries");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(outer_dict->elements[0].value.get())->value, "3");

        // Pair 1: flags: { ... }
        EXPECT_EQ(outer_dict->elements[1].key, "flags");
        auto inner_dict = dynamic_cast<DictLiteral *>(outer_dict->elements[1].value.get());
        ASSERT_NE(inner_dict, nullptr);
        ASSERT_EQ(inner_dict->elements.size(), 2);

        EXPECT_EQ(inner_dict->elements[0].key, "active");
        EXPECT_EQ(dynamic_cast<BooleanLiteral*>(inner_dict->elements[0].value.get())->value, true);

        EXPECT_EQ(inner_dict->elements[1].key, "safe");
        EXPECT_EQ(dynamic_cast<BooleanLiteral*>(inner_dict->elements[1].value.get())->value, false);
    }

    TEST_F(AstBaseTest, ValidatesTupleLiteralInModifier) {
        auto ast = parse_code("@position(coords: (10.5, -20.0, 5)) let obj = 0");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        const auto &mod = assign_node->modifiers[0];

        EXPECT_EQ(mod.arguments[0].first, "coords");
        auto tuple_lit = dynamic_cast<TupleLiteral *>(mod.arguments[0].second.get());
        ASSERT_NE(tuple_lit, nullptr);
        ASSERT_EQ(tuple_lit->elements.size(), 3);

        EXPECT_EQ(dynamic_cast<NumberLiteral*>(tuple_lit->elements[0].get())->value, "10.5");

        auto unary_minus = dynamic_cast<UnaryExpression *>(tuple_lit->elements[1].get());
        ASSERT_NE(unary_minus, nullptr);
        EXPECT_EQ(unary_minus->op, TokenType::Minus);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(unary_minus->right.get())->value, "20.0");

        EXPECT_EQ(dynamic_cast<NumberLiteral*>(tuple_lit->elements[2].get())->value, "5");
    }

    TEST_F(AstBaseTest, ValidatesNestedTensorLiteralInModifier) {
        auto ast = parse_code("@kernel(weights: [[1, 0], [0, 1]]) func process() -> void {}");
        auto func_node = ast->function_definitions[0].get();
        const auto &mod = func_node->modifiers[0];

        EXPECT_EQ(mod.arguments[0].first, "weights");
        auto outer_tensor = dynamic_cast<TensorLiteral *>(mod.arguments[0].second.get());
        ASSERT_NE(outer_tensor, nullptr);
        ASSERT_EQ(outer_tensor->elements.size(), 2);

        // Inner Tensor 1: [1, 0]
        auto inner_tensor_1 = dynamic_cast<TensorLiteral *>(outer_tensor->elements[0].get());
        ASSERT_NE(inner_tensor_1, nullptr);
        ASSERT_EQ(inner_tensor_1->elements.size(), 2);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner_tensor_1->elements[0].get())->value, "1");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner_tensor_1->elements[1].get())->value, "0");

        // Inner Tensor 2: [0, 1]
        auto inner_tensor_2 = dynamic_cast<TensorLiteral *>(outer_tensor->elements[1].get());
        ASSERT_NE(inner_tensor_2, nullptr);
        ASSERT_EQ(inner_tensor_2->elements.size(), 2);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner_tensor_2->elements[0].get())->value, "0");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner_tensor_2->elements[1].get())->value, "1");
    }

    TEST_F(AstBaseTest, ValidatesDeepDotAccessInModifier) {
        auto ast = parse_code("@ui(theme: App.Theme.Dark, icon: Icons.Warning) struct Window {}");
        auto struct_node = ast->struct_definitions[0].get();
        const auto &mod = struct_node->modifiers[0];

        ASSERT_EQ(mod.arguments.size(), 2);

        // Arg 0: App.Theme.Dark
        EXPECT_EQ(mod.arguments[0].first, "theme");
        auto dot1 = dynamic_cast<DotAccess *>(mod.arguments[0].second.get());
        ASSERT_NE(dot1, nullptr);
        EXPECT_EQ(dot1->property_name, "Dark");

        auto dot2 = dynamic_cast<DotAccess *>(dot1->target.get());
        ASSERT_NE(dot2, nullptr);
        EXPECT_EQ(dot2->property_name, "Theme");

        auto id = dynamic_cast<IdentifierAccess *>(dot2->target.get());
        ASSERT_NE(id, nullptr);
        EXPECT_EQ(id->name, "App");

        // Arg 1: Icons.Warning
        EXPECT_EQ(mod.arguments[1].first, "icon");
        auto icon_dot = dynamic_cast<DotAccess *>(mod.arguments[1].second.get());
        ASSERT_NE(icon_dot, nullptr);
        EXPECT_EQ(icon_dot->property_name, "Warning");
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(icon_dot->target.get())->name, "Icons");
    }

    TEST_F(AstBaseTest, ValidatesBracketAccessAndSlicingInModifier) {
        auto ast = parse_code("@memory(chunk: buffer[0:256], offset: limits[5]) let block = 0");
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        const auto &mod = assign_node->modifiers[0];

        ASSERT_EQ(mod.arguments.size(), 2);

        // Arg 0: buffer[0:256] (Slicing)
        EXPECT_EQ(mod.arguments[0].first, "chunk");
        auto slice_access = dynamic_cast<BracketAccess *>(mod.arguments[0].second.get());
        ASSERT_NE(slice_access, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(slice_access->target.get())->name, "buffer");

        auto slice_bin = dynamic_cast<BinaryExpression *>(slice_access->index.get());
        ASSERT_NE(slice_bin, nullptr);
        EXPECT_EQ(slice_bin->op, TokenType::Colon);
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(slice_bin->left.get())->value, "0");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(slice_bin->right.get())->value, "256");

        // Arg 1: limits[5] (Single Index)
        EXPECT_EQ(mod.arguments[1].first, "offset");
        auto index_access = dynamic_cast<BracketAccess *>(mod.arguments[1].second.get());
        ASSERT_NE(index_access, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(index_access->target.get())->name, "limits");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(index_access->index.get())->value, "5");
    }

    TEST_F(AstBaseTest, ValidatesFunctionCallWithArgumentsInModifier) {
        auto ast = parse_code("@retry(strategy: exponential_backoff(base: 2, max: 10)) func fetch() -> void {}");
        auto func_node = ast->function_definitions[0].get();
        const auto &mod = func_node->modifiers[0];

        EXPECT_EQ(mod.arguments[0].first, "strategy");
        auto func_call = dynamic_cast<FunctionCall *>(mod.arguments[0].second.get());
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
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        const auto &mod = assign_node->modifiers[0];

        // payload: { ... }
        EXPECT_EQ(mod.arguments[0].first, "payload");
        auto payload_dict = dynamic_cast<DictLiteral *>(mod.arguments[0].second.get());
        ASSERT_NE(payload_dict, nullptr);
        ASSERT_EQ(payload_dict->elements.size(), 2);

        // data: fetch(args: (...))
        EXPECT_EQ(payload_dict->elements[0].key, "data");
        auto fetch_call = dynamic_cast<FunctionCall *>(payload_dict->elements[0].value.get());
        ASSERT_NE(fetch_call, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(fetch_call->target.get())->name, "fetch");
        ASSERT_EQ(fetch_call->arguments.size(), 1);

        // args: (State.Active, buffer[1])
        EXPECT_EQ(fetch_call->arguments[0].first, "args");
        auto args_tuple = dynamic_cast<TupleLiteral *>(fetch_call->arguments[0].second.get());
        ASSERT_NE(args_tuple, nullptr);
        ASSERT_EQ(args_tuple->elements.size(), 2);

        // Tuple element 0: State.Active
        auto state_dot = dynamic_cast<DotAccess *>(args_tuple->elements[0].get());
        ASSERT_NE(state_dot, nullptr);
        EXPECT_EQ(state_dot->property_name, "Active");
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(state_dot->target.get())->name, "State");

        // Tuple element 1: buffer[1]
        auto buffer_bracket = dynamic_cast<BracketAccess *>(args_tuple->elements[1].get());
        ASSERT_NE(buffer_bracket, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess*>(buffer_bracket->target.get())->name, "buffer");
        EXPECT_EQ(dynamic_cast<NumberLiteral*>(buffer_bracket->index.get())->value, "1");

        // fallback: [ [1], [2] ]
        EXPECT_EQ(payload_dict->elements[1].key, "fallback");
        auto fallback_tensor = dynamic_cast<TensorLiteral *>(payload_dict->elements[1].value.get());
        ASSERT_NE(fallback_tensor, nullptr);
        ASSERT_EQ(fallback_tensor->elements.size(), 2);
    }

    TEST_F(AstBaseTest, ValidatesMultipleModifiersWithComplexParamsStressTest) {
        std::string code =
                "@export @correlated(with: [ { name: erp, direction: CorrelationDirection.Negative } ]) let rf = 4.5%\n"
                "@export @correlated(with: [ { name: rf, direction: CorrelationDirection.Negative } ]) let erp = 5%";

        auto ast = parse_code(code);
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        EXPECT_EQ(assign_node->modifiers.size(), 2);


        const auto &mod_1 = assign_node->modifiers[0];
        EXPECT_EQ(mod_1.arguments.size(), 0);
        EXPECT_EQ(mod_1.name, "export");

        const auto &mod_2 = assign_node->modifiers[1];
        EXPECT_EQ(mod_2.arguments.size(), 1);
        EXPECT_EQ(mod_2.name, "correlated");
        EXPECT_EQ(mod_2.arguments[0].first, "with");
        auto with_array = dynamic_cast<TensorLiteral *>(mod_2.arguments[0].second.get());
        ASSERT_NE(with_array, nullptr);
        ASSERT_EQ(with_array->elements.size(), 1);

        auto correlation_1_dict = dynamic_cast<DictLiteral *>(with_array->elements[0].get());
        ASSERT_NE(correlation_1_dict, nullptr);
        EXPECT_EQ(correlation_1_dict->elements.size(), 2);
        EXPECT_EQ(correlation_1_dict->elements[0].key, "name");
        auto correlation_1_dict_name = dynamic_cast<IdentifierAccess *>(correlation_1_dict->elements[0].value.get());
        EXPECT_EQ(correlation_1_dict_name->name, "erp");
        auto correlation_1_dict_direction = dynamic_cast<DotAccess *>(correlation_1_dict->elements[1].value.get());
        EXPECT_NE(correlation_1_dict_direction, nullptr);
        EXPECT_EQ(correlation_1_dict_direction->property_name, "Negative");
        auto correlation_1_dict_direction_enum = dynamic_cast<IdentifierAccess *>(correlation_1_dict_direction->target.
            get());
        EXPECT_EQ(correlation_1_dict_direction_enum->name, "CorrelationDirection");

        EXPECT_FALSE(assign_node->is_mutable);
        EXPECT_EQ(assign_node->targets.size(), 1);
        EXPECT_EQ(assign_node->targets[0].first, "rf");
        EXPECT_EQ(assign_node->targets[0].second, nullptr);

        auto value = dynamic_cast<PercentageLiteral *>(assign_node->value.get());
        EXPECT_EQ(value->value, "4.5%");
    }

    TEST_F(AstBaseTest, ValidatesModifiersOnDictLiteralField) {
        std::string code =
                "@scenario(type: \"base\")\n"
                "let gcp_segment: Segment = {\n"
                "@correlated(with: [ { name: market_size, direction: CorrelationDirection.Negative } ])"
                "market_share: 11%,\n"
                "market_size: 13_624 / 11% * 4 }";

        auto ast = parse_code(code);
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        EXPECT_EQ(assign_node->modifiers.size(), 1);


        const auto &assign_mod_1 = assign_node->modifiers[0];
        EXPECT_EQ(assign_mod_1.arguments.size(), 1);
        EXPECT_EQ(assign_mod_1.name, "scenario");
        EXPECT_EQ(assign_mod_1.arguments[0].first, "type");
        auto type_arg = dynamic_cast<StringLiteral *>(assign_mod_1.arguments[0].second.get());
        ASSERT_EQ(type_arg->value, "\"base\"");


        const auto assign_node_value = dynamic_cast<DictLiteral *>(assign_node->value.get());
        ASSERT_NE(assign_node_value, nullptr);

        ASSERT_EQ(assign_node_value->elements.size(), 2);

        const auto &dict_elem_1 = assign_node_value->elements[0];
        ASSERT_EQ(dict_elem_1.modifiers.size(), 1);
        ASSERT_EQ(dict_elem_1.modifiers[0].name, "correlated");
        ASSERT_EQ(dict_elem_1.modifiers[0].arguments.size(), 1);

        const auto &mod_1 = dict_elem_1.modifiers[0];
        EXPECT_EQ(mod_1.arguments.size(), 1);
        EXPECT_EQ(mod_1.name, "correlated");
        EXPECT_EQ(mod_1.arguments[0].first, "with");
        auto mod_first_param = dynamic_cast<TensorLiteral *>(mod_1.arguments[0].second.get());
        ASSERT_NE(mod_first_param, nullptr);
        ASSERT_EQ(mod_first_param->elements.size(), 1);

        auto correlation_1_dict = dynamic_cast<DictLiteral *>(mod_first_param->elements[0].get());
        ASSERT_NE(correlation_1_dict, nullptr);
        EXPECT_EQ(correlation_1_dict->elements.size(), 2);
        EXPECT_EQ(correlation_1_dict->elements[0].key, "name");
        auto correlation_1_dict_name = dynamic_cast<IdentifierAccess *>(correlation_1_dict->elements[0].value.get());
        EXPECT_EQ(correlation_1_dict_name->name, "market_size");
        auto correlation_1_dict_direction = dynamic_cast<DotAccess *>(correlation_1_dict->elements[1].value.get());
        EXPECT_NE(correlation_1_dict_direction, nullptr);
        EXPECT_EQ(correlation_1_dict_direction->property_name, "Negative");
        auto correlation_1_dict_direction_enum = dynamic_cast<IdentifierAccess *>(correlation_1_dict_direction->target.
            get());
        EXPECT_EQ(correlation_1_dict_direction_enum->name, "CorrelationDirection");

        auto dict_elem_1_value = dynamic_cast<PercentageLiteral *>(dict_elem_1.value.get());
        EXPECT_EQ(dict_elem_1_value->value, "11%");


        const auto &dict_elem_2 = assign_node_value->elements[1];
        ASSERT_EQ(dict_elem_2.modifiers.size(), 0);

        auto dict_elem_2_value = dynamic_cast<BinaryExpression *>(dict_elem_2.value.get());
        EXPECT_NE(dict_elem_2_value, nullptr);
    }

    TEST_F(AstBaseTest, ValidatesSimpleModifierOnDictLiteralKey) {
        auto ast = parse_code("let config = { @optional port: 8080 }");

        ASSERT_EQ(ast->execution_steps.size(), 1);
        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_NE(assign_node, nullptr);

        auto dict_lit = dynamic_cast<DictLiteral *>(assign_node->value.get());
        ASSERT_NE(dict_lit, nullptr);
        ASSERT_EQ(dict_lit->elements.size(), 1);

        const auto &dict_item = dict_lit->elements[0];
        EXPECT_EQ(dict_item.key, "port");

        // Check modifiers on the key
        ASSERT_EQ(dict_item.modifiers.size(), 1);
        EXPECT_EQ(dict_item.modifiers[0].name, "optional");
        EXPECT_TRUE(dict_item.modifiers[0].arguments.empty());

        auto value = dynamic_cast<NumberLiteral *>(dict_item.value.get());
        ASSERT_NE(value, nullptr);
        EXPECT_EQ(value->value, "8080");
    }

    TEST_F(AstBaseTest, ValidatesMultipleModifiersWithArgsOnDictLiteralKey) {
        auto ast = parse_code(
            "let schema = { \n"
            "@rename(to: \"user_id\") @indexed(unique: true) id: 123, \n"
            "@transient session: \"abc\" \n"
            "}");

        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        auto dict_lit = dynamic_cast<DictLiteral *>(assign_node->value.get());
        ASSERT_NE(dict_lit, nullptr);
        ASSERT_EQ(dict_lit->elements.size(), 2);

        // --- First element: id ---
        const auto &id_item = dict_lit->elements[0];
        EXPECT_EQ(id_item.key, "id");
        ASSERT_EQ(id_item.modifiers.size(), 2);

        // Mod 0: @rename(to: "user_id")
        EXPECT_EQ(id_item.modifiers[0].name, "rename");
        ASSERT_EQ(id_item.modifiers[0].arguments.size(), 1);
        EXPECT_EQ(id_item.modifiers[0].arguments[0].first, "to");
        EXPECT_EQ(dynamic_cast<StringLiteral *>(id_item.modifiers[0].arguments[0].second.get())->value, "\"user_id\"");

        // Mod 1: @indexed(unique: true)
        EXPECT_EQ(id_item.modifiers[1].name, "indexed");
        ASSERT_EQ(id_item.modifiers[1].arguments.size(), 1);
        EXPECT_EQ(id_item.modifiers[1].arguments[0].first, "unique");
        EXPECT_EQ(dynamic_cast<BooleanLiteral *>(id_item.modifiers[1].arguments[0].second.get())->value, true);

        // --- Second element: session ---
        const auto &session_item = dict_lit->elements[1];
        EXPECT_EQ(session_item.key, "session");
        ASSERT_EQ(session_item.modifiers.size(), 1);
        EXPECT_EQ(session_item.modifiers[0].name, "transient");
        EXPECT_TRUE(session_item.modifiers[0].arguments.empty());
    }

    TEST_F(AstBaseTest, ValidatesDictLiteralWithMixedModifiedAndUnmodifiedKeys) {
        auto ast = parse_code(
            "let data = { "
            "normal_key: 1, "
            "@special flagged_key: 2, "
            "another_normal: 3 "
            "}");

        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        auto dict_lit = dynamic_cast<DictLiteral *>(assign_node->value.get());
        ASSERT_NE(dict_lit, nullptr);
        ASSERT_EQ(dict_lit->elements.size(), 3);

        EXPECT_TRUE(dict_lit->elements[0].modifiers.empty());

        ASSERT_EQ(dict_lit->elements[1].modifiers.size(), 1);
        EXPECT_EQ(dict_lit->elements[1].modifiers[0].name, "special");

        EXPECT_TRUE(dict_lit->elements[2].modifiers.empty());
    }

    // =========================================================================
    // ENUMERATION CASE MODIFIER TESTS
    // =========================================================================

    TEST_F(AstBaseTest, ValidatesSimpleModifierOnEnumCase) {
        auto ast = parse_code("enum Status: int { @primary Active, Inactive }");

        ASSERT_EQ(ast->enum_definitions.size(), 1);
        auto enum_node = ast->enum_definitions[0].get();
        EXPECT_EQ(enum_node->name, "Status");

        ASSERT_EQ(enum_node->cases.size(), 2);

        // Case 0: Active (with modifier)
        const auto &case_active = enum_node->cases[0];
        EXPECT_EQ(case_active.name, "Active");
        EXPECT_EQ(case_active.value, nullptr); // No value assigned

        ASSERT_EQ(case_active.modifiers.size(), 1);
        EXPECT_EQ(case_active.modifiers[0].name, "primary");
        EXPECT_TRUE(case_active.modifiers[0].arguments.empty());

        // Case 1: Inactive (no modifier)
        const auto &case_inactive = enum_node->cases[1];
        EXPECT_EQ(case_inactive.name, "Inactive");
        EXPECT_TRUE(case_inactive.modifiers.empty());
    }

    TEST_F(AstBaseTest, ValidatesModifiersWithArgsOnEnumCaseWithValues) {
        auto ast = parse_code(
            "enum ErrorCode: int { "
            "@alias(name: \"NOT_FOUND\") NotFound = 404, "
            "@deprecated(since: \"v1.2\") ServerError = 500 "
            "}");

        auto enum_node = ast->enum_definitions[0].get();
        ASSERT_EQ(enum_node->cases.size(), 2);

        // --- Case 0: NotFound ---
        const auto &not_found = enum_node->cases[0];
        EXPECT_EQ(not_found.name, "NotFound");

        ASSERT_EQ(not_found.modifiers.size(), 1);
        EXPECT_EQ(not_found.modifiers[0].name, "alias");
        ASSERT_EQ(not_found.modifiers[0].arguments.size(), 1);
        EXPECT_EQ(not_found.modifiers[0].arguments[0].first, "name");
        EXPECT_EQ(dynamic_cast<StringLiteral *>(not_found.modifiers[0].arguments[0].second.get())->value,
                  "\"NOT_FOUND\"");

        auto val_404 = dynamic_cast<NumberLiteral *>(not_found.value.get());
        ASSERT_NE(val_404, nullptr);
        EXPECT_EQ(val_404->value, "404");

        // --- Case 1: ServerError ---
        const auto &server_error = enum_node->cases[1];
        EXPECT_EQ(server_error.name, "ServerError");

        ASSERT_EQ(server_error.modifiers.size(), 1);
        EXPECT_EQ(server_error.modifiers[0].name, "deprecated");
        ASSERT_EQ(server_error.modifiers[0].arguments.size(), 1);
        EXPECT_EQ(server_error.modifiers[0].arguments[0].first, "since");
        EXPECT_EQ(dynamic_cast<StringLiteral *>(server_error.modifiers[0].arguments[0].second.get())->value,
                  "\"v1.2\"");

        auto val_500 = dynamic_cast<NumberLiteral *>(server_error.value.get());
        ASSERT_NE(val_500, nullptr);
        EXPECT_EQ(val_500->value, "500");
    }

    TEST_F(AstBaseTest, ValidatesMultipleModifiersOnEnumCaseAndExpressions) {
        auto ast = parse_code(
            "enum Flags: int { "
            "@hidden @bitwise(shift: 1) Read = 1 * 2, "
            "@bitwise(shift: 2) Write = 1 * 4 "
            "}");

        auto enum_node = ast->enum_definitions[0].get();
        ASSERT_EQ(enum_node->cases.size(), 2);

        // --- Case 0: Read ---
        const auto &read_case = enum_node->cases[0];
        EXPECT_EQ(read_case.name, "Read");
        ASSERT_EQ(read_case.modifiers.size(), 2);

        // Mod 0: hidden
        EXPECT_EQ(read_case.modifiers[0].name, "hidden");
        EXPECT_TRUE(read_case.modifiers[0].arguments.empty());

        // Mod 1: bitwise
        EXPECT_EQ(read_case.modifiers[1].name, "bitwise");
        EXPECT_EQ(read_case.modifiers[1].arguments[0].first, "shift");
        EXPECT_EQ(dynamic_cast<NumberLiteral *>(read_case.modifiers[1].arguments[0].second.get())->value, "1");

        // Value: 1 * 2
        auto bin_expr1 = dynamic_cast<BinaryExpression *>(read_case.value.get());
        ASSERT_NE(bin_expr1, nullptr);
        EXPECT_EQ(bin_expr1->op, TokenType::Star);
        EXPECT_EQ(dynamic_cast<NumberLiteral *>(bin_expr1->left.get())->value, "1");
        EXPECT_EQ(dynamic_cast<NumberLiteral *>(bin_expr1->right.get())->value, "2");

        // --- Case 1: Write ---
        const auto &write_case = enum_node->cases[1];
        EXPECT_EQ(write_case.name, "Write");
        ASSERT_EQ(write_case.modifiers.size(), 1);
        EXPECT_EQ(write_case.modifiers[0].name, "bitwise");
    }

    TEST_F(AstBaseTest, ValidatesEnumCaseWithComplexModifierDictAndArray) {
        auto ast = parse_code(
            "enum Events: int { "
            "@audit(tags:[\"auth\", \"login\"], meta: { severity: 1 }) LoginEvent "
            "}");

        auto enum_node = ast->enum_definitions[0].get();
        ASSERT_EQ(enum_node->cases.size(), 1);

        const auto &login_case = enum_node->cases[0];
        EXPECT_EQ(login_case.name, "LoginEvent");
        ASSERT_EQ(login_case.modifiers.size(), 1);

        const auto &audit_mod = login_case.modifiers[0];
        EXPECT_EQ(audit_mod.name, "audit");
        ASSERT_EQ(audit_mod.arguments.size(), 2);

        // Arg 0: tags: ["auth", "login"]
        EXPECT_EQ(audit_mod.arguments[0].first, "tags");
        auto tags_tensor = dynamic_cast<TensorLiteral *>(audit_mod.arguments[0].second.get());
        ASSERT_NE(tags_tensor, nullptr);
        ASSERT_EQ(tags_tensor->elements.size(), 2);
        EXPECT_EQ(dynamic_cast<StringLiteral *>(tags_tensor->elements[0].get())->value, "\"auth\"");

        // Arg 1: meta: { severity: 1 }
        EXPECT_EQ(audit_mod.arguments[1].first, "meta");
        auto meta_dict = dynamic_cast<DictLiteral *>(audit_mod.arguments[1].second.get());
        ASSERT_NE(meta_dict, nullptr);
        ASSERT_EQ(meta_dict->elements.size(), 1);
        EXPECT_EQ(meta_dict->elements[0].key, "severity");
        EXPECT_EQ(dynamic_cast<NumberLiteral *>(meta_dict->elements[0].value.get())->value, "1");
    }

    TEST_F(AstBaseTest, ValidatesNestedDictsWithModifiedKeys) {
        auto ast = parse_code(
            "let payload = { "
            "  @root config: { "
            "    @nested retries: 3 "
            "  } "
            "}");

        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        auto outer_dict = dynamic_cast<DictLiteral *>(assign_node->value.get());
        ASSERT_NE(outer_dict, nullptr);
        ASSERT_EQ(outer_dict->elements.size(), 1);

        // Check Outer Dict Key
        const auto &config_item = outer_dict->elements[0];
        EXPECT_EQ(config_item.key, "config");
        ASSERT_EQ(config_item.modifiers.size(), 1);
        EXPECT_EQ(config_item.modifiers[0].name, "root");

        // Check Inner Dict
        auto inner_dict = dynamic_cast<DictLiteral *>(config_item.value.get());
        ASSERT_NE(inner_dict, nullptr);
        ASSERT_EQ(inner_dict->elements.size(), 1);

        // Check Inner Dict Key
        const auto &retries_item = inner_dict->elements[0];
        EXPECT_EQ(retries_item.key, "retries");
        ASSERT_EQ(retries_item.modifiers.size(), 1);
        EXPECT_EQ(retries_item.modifiers[0].name, "nested");
        EXPECT_EQ(dynamic_cast<NumberLiteral *>(retries_item.value.get())->value, "3");
    }

    TEST_F(AstBaseTest, ValidatesModifierWithComplexExpressionsOnDictKey) {
        auto ast = parse_code(
            "let obj = { "
            "@computed(val: 10 * 2, fn: get_hash()) id: 0 "
            "}");

        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        auto dict_lit = dynamic_cast<DictLiteral *>(assign_node->value.get());

        const auto &id_item = dict_lit->elements[0];
        ASSERT_EQ(id_item.modifiers.size(), 1);
        const auto &mod = id_item.modifiers[0];

        EXPECT_EQ(mod.name, "computed");
        ASSERT_EQ(mod.arguments.size(), 2);

        // Arg 0: val: 10 * 2 (BinaryExpression)
        EXPECT_EQ(mod.arguments[0].first, "val");
        auto math_expr = dynamic_cast<BinaryExpression *>(mod.arguments[0].second.get());
        ASSERT_NE(math_expr, nullptr);
        EXPECT_EQ(math_expr->op, TokenType::Star);
        EXPECT_EQ(dynamic_cast<NumberLiteral *>(math_expr->left.get())->value, "10");
        EXPECT_EQ(dynamic_cast<NumberLiteral *>(math_expr->right.get())->value, "2");

        // Arg 1: fn: get_hash() (FunctionCall)
        EXPECT_EQ(mod.arguments[1].first, "fn");
        auto call_expr = dynamic_cast<FunctionCall *>(mod.arguments[1].second.get());
        ASSERT_NE(call_expr, nullptr);
        EXPECT_EQ(dynamic_cast<IdentifierAccess *>(call_expr->target.get())->name, "get_hash");
    }

    TEST_F(AstBaseTest, ValidatesModifierWithEmptyParensAndTrailingCommaOnDictKey) {
        auto ast = parse_code(
            "let meta = { "
            "@internal() data: 1, "
            "}");

        auto assign_node = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        auto dict_lit = dynamic_cast<DictLiteral *>(assign_node->value.get());
        ASSERT_NE(dict_lit, nullptr);
        ASSERT_EQ(dict_lit->elements.size(), 1);

        const auto &item = dict_lit->elements[0];
        ASSERT_EQ(item.modifiers.size(), 1);
        EXPECT_EQ(item.modifiers[0].name, "internal");
        EXPECT_TRUE(item.modifiers[0].arguments.empty());
    }

    TEST_F(AstBaseTest, ValidatesModifiersOnBothEnumAndCases) {
        auto ast = parse_code(
            "@export @api(version: 2) "
            "enum Config: int { "
            "  @deprecated Old = 0, "
            "  @latest New = 1 "
            "}");

        ASSERT_EQ(ast->enum_definitions.size(), 1);
        auto enum_node = ast->enum_definitions[0].get();

        // 1. Verify Enum-level Modifiers
        ASSERT_EQ(enum_node->modifiers.size(), 2);
        EXPECT_EQ(enum_node->modifiers[0].name, "export");
        EXPECT_EQ(enum_node->modifiers[1].name, "api");
        EXPECT_EQ(enum_node->modifiers[1].arguments[0].first, "version");

        // 2. Verify Case-level Modifiers
        ASSERT_EQ(enum_node->cases.size(), 2);

        const auto &old_case = enum_node->cases[0];
        EXPECT_EQ(old_case.name, "Old");
        ASSERT_EQ(old_case.modifiers.size(), 1);
        EXPECT_EQ(old_case.modifiers[0].name, "deprecated");

        const auto &new_case = enum_node->cases[1];
        EXPECT_EQ(new_case.name, "New");
        ASSERT_EQ(new_case.modifiers.size(), 1);
        EXPECT_EQ(new_case.modifiers[0].name, "latest");
    }

    TEST_F(AstBaseTest, ValidatesEnumCaseModifierWithDotAccessArg) {
        auto ast = parse_code(
            "enum Mode: int { "
            "@fallback(to: System.Default) Custom = 1 "
            "}");

        auto enum_node = ast->enum_definitions[0].get();
        const auto &custom_case = enum_node->cases[0];

        ASSERT_EQ(custom_case.modifiers.size(), 1);
        const auto &mod = custom_case.modifiers[0];

        EXPECT_EQ(mod.name, "fallback");
        ASSERT_EQ(mod.arguments.size(), 1);
        EXPECT_EQ(mod.arguments[0].first, "to");

        auto dot_access = dynamic_cast<DotAccess *>(mod.arguments[0].second.get());
        ASSERT_NE(dot_access, nullptr);
        EXPECT_EQ(dot_access->property_name, "Default");
        EXPECT_EQ(dynamic_cast<IdentifierAccess *>(dot_access->target.get())->name, "System");
    }

    TEST_F(AstBaseTest, ValidatesEmptyParensAndTrailingCommaOnEnumCase) {
        auto ast = parse_code(
            "enum State: int { "
            "@init() Start,"
            "}");

        auto enum_node = ast->enum_definitions[0].get();
        ASSERT_EQ(enum_node->cases.size(), 1);

        const auto &start_case = enum_node->cases[0];
        EXPECT_EQ(start_case.name, "Start");
        EXPECT_EQ(start_case.value, nullptr);

        ASSERT_EQ(start_case.modifiers.size(), 1);
        EXPECT_EQ(start_case.modifiers[0].name, "init");
        EXPECT_TRUE(start_case.modifiers[0].arguments.empty());
    }

    TEST_F(AstBaseTest, ValidatesFunctionParametersWithoutModifiers) {
        auto ast = parse_code("func add(a: int, b: int) -> int {}");

        ASSERT_EQ(ast->function_definitions.size(), 1);
        auto func_node = ast->function_definitions[0].get();

        ASSERT_EQ(func_node->parameters.size(), 2);

        EXPECT_EQ(func_node->parameters[0].name, "a");
        EXPECT_TRUE(func_node->parameters[0].modifiers.empty());

        EXPECT_EQ(func_node->parameters[1].name, "b");
        EXPECT_TRUE(func_node->parameters[1].modifiers.empty());
    }

    TEST_F(AstBaseTest, ValidatesSimpleModifierOnFunctionParameter) {
        auto ast = parse_code("func update(@mut value: int) -> void {}");

        ASSERT_EQ(ast->function_definitions.size(), 1);
        auto func_node = ast->function_definitions[0].get();

        ASSERT_EQ(func_node->parameters.size(), 1);
        const auto &param = func_node->parameters[0];

        EXPECT_EQ(param.name, "value");
        ASSERT_EQ(param.modifiers.size(), 1);

        EXPECT_EQ(param.modifiers[0].name, "mut");
        EXPECT_TRUE(param.modifiers[0].arguments.empty());
    }

    TEST_F(AstBaseTest, ValidatesMultipleFunctionParametersWithModifiers) {
        auto ast = parse_code("func swap(@ref a: int, @ref b: int, @unused ctx: context) -> void {}");

        ASSERT_EQ(ast->function_definitions.size(), 1);
        auto func_node = ast->function_definitions[0].get();

        ASSERT_EQ(func_node->parameters.size(), 3);

        // Param 0: @ref a
        EXPECT_EQ(func_node->parameters[0].name, "a");
        ASSERT_EQ(func_node->parameters[0].modifiers.size(), 1);
        EXPECT_EQ(func_node->parameters[0].modifiers[0].name, "ref");

        // Param 1: @ref b
        EXPECT_EQ(func_node->parameters[1].name, "b");
        ASSERT_EQ(func_node->parameters[1].modifiers.size(), 1);
        EXPECT_EQ(func_node->parameters[1].modifiers[0].name, "ref");

        // Param 2: @unused ctx
        EXPECT_EQ(func_node->parameters[2].name, "ctx");
        ASSERT_EQ(func_node->parameters[2].modifiers.size(), 1);
        EXPECT_EQ(func_node->parameters[2].modifiers[0].name, "unused");
    }

    TEST_F(AstBaseTest, ValidatesModifierWithArgumentsOnFunctionParameter) {
        auto ast = parse_code("func set_age(@clamp(min: 0, max: 120) age: int) -> void {}");

        ASSERT_EQ(ast->function_definitions.size(), 1);
        auto func_node = ast->function_definitions[0].get();

        ASSERT_EQ(func_node->parameters.size(), 1);
        const auto &param = func_node->parameters[0];

        EXPECT_EQ(param.name, "age");
        ASSERT_EQ(param.modifiers.size(), 1);

        const auto &mod = param.modifiers[0];
        EXPECT_EQ(mod.name, "clamp");
        ASSERT_EQ(mod.arguments.size(), 2);

        // Arg 0: min: 0
        EXPECT_EQ(mod.arguments[0].first, "min");
        auto min_val = dynamic_cast<NumberLiteral *>(mod.arguments[0].second.get());
        ASSERT_NE(min_val, nullptr);
        EXPECT_EQ(min_val->value, "0");

        // Arg 1: max: 120
        EXPECT_EQ(mod.arguments[1].first, "max");
        auto max_val = dynamic_cast<NumberLiteral *>(mod.arguments[1].second.get());
        ASSERT_NE(max_val, nullptr);
        EXPECT_EQ(max_val->value, "120");
    }

    TEST_F(AstBaseTest, ValidatesStackedModifiersOnFunctionParameter) {
        auto ast = parse_code("func process(@mut @log(level: \"debug\") @inject() data: string) -> void {}");

        ASSERT_EQ(ast->function_definitions.size(), 1);
        auto func_node = ast->function_definitions[0].get();

        ASSERT_EQ(func_node->parameters.size(), 1);
        const auto &param = func_node->parameters[0];

        EXPECT_EQ(param.name, "data");
        ASSERT_EQ(param.modifiers.size(), 3);

        // Mod 0: @mut
        EXPECT_EQ(param.modifiers[0].name, "mut");
        EXPECT_TRUE(param.modifiers[0].arguments.empty());

        // Mod 1: @log(level: "debug")
        EXPECT_EQ(param.modifiers[1].name, "log");
        ASSERT_EQ(param.modifiers[1].arguments.size(), 1);
        EXPECT_EQ(param.modifiers[1].arguments[0].first, "level");
        EXPECT_EQ(dynamic_cast<StringLiteral *>(param.modifiers[1].arguments[0].second.get())->value, "\"debug\"");

        // Mod 2: @inject()
        EXPECT_EQ(param.modifiers[2].name, "inject");
        EXPECT_TRUE(param.modifiers[2].arguments.empty());
    }

    TEST_F(AstBaseTest, ValidatesModifierWithComplexArgumentsOnFunctionParameter) {
        auto ast = parse_code(
            "func route(@meta(headers: { auth: true }, tags: [\"api\", \"v1\"]) req: Request) -> void {}"
        );

        ASSERT_EQ(ast->function_definitions.size(), 1);
        auto func_node = ast->function_definitions[0].get();

        ASSERT_EQ(func_node->parameters.size(), 1);
        const auto &param = func_node->parameters[0];
        ASSERT_EQ(param.modifiers.size(), 1);

        const auto &mod = param.modifiers[0];
        EXPECT_EQ(mod.name, "meta");
        ASSERT_EQ(mod.arguments.size(), 2);

        // Arg 0: headers: { auth: true }
        EXPECT_EQ(mod.arguments[0].first, "headers");
        auto dict_val = dynamic_cast<DictLiteral *>(mod.arguments[0].second.get());
        ASSERT_NE(dict_val, nullptr);
        ASSERT_EQ(dict_val->elements.size(), 1);
        EXPECT_EQ(dict_val->elements[0].key, "auth");
        EXPECT_EQ(dynamic_cast<BooleanLiteral *>(dict_val->elements[0].value.get())->value, true);

        // Arg 1: tags: ["api", "v1"]
        EXPECT_EQ(mod.arguments[1].first, "tags");
        auto tensor_val = dynamic_cast<TensorLiteral *>(mod.arguments[1].second.get());
        ASSERT_NE(tensor_val, nullptr);
        ASSERT_EQ(tensor_val->elements.size(), 2);
        EXPECT_EQ(dynamic_cast<StringLiteral *>(tensor_val->elements[0].get())->value, "\"api\"");
        EXPECT_EQ(dynamic_cast<StringLiteral *>(tensor_val->elements[1].get())->value, "\"v1\"");
    }

    TEST_F(AstBaseTest, ValidatesFunctionLevelAndParameterLevelModifiersSeparately) {
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

    TEST_F(AstBaseTest, ValidatesModifierOnFunctionParameterWithDefaultValue) {
        // Ensure modifiers don't break default parameter value parsing
        auto ast = parse_code("func connect(@timeout(ms: 500) duration: int = 1000) -> void {}");

        ASSERT_EQ(ast->function_definitions.size(), 1);
        auto func_node = ast->function_definitions[0].get();

        ASSERT_EQ(func_node->parameters.size(), 1);
        const auto &param = func_node->parameters[0];

        EXPECT_EQ(param.name, "duration");

        // Verify Modifier
        ASSERT_EQ(param.modifiers.size(), 1);
        EXPECT_EQ(param.modifiers[0].name, "timeout");

        // Verify Default Value
        ASSERT_NE(param.default_value, nullptr);
        auto default_val = dynamic_cast<NumberLiteral *>(param.default_value.get());
        ASSERT_NE(default_val, nullptr);
        EXPECT_EQ(default_val->value, "1000");
    }

    TEST_F(AstBaseTest, ValidatesStructWithoutAnyModifiers) {
        auto ast = parse_code("struct Basic { x: int, y: float }");

        ASSERT_EQ(ast->struct_definitions.size(), 1);
        auto node = ast->struct_definitions[0].get();

        EXPECT_EQ(node->name, "Basic");
        EXPECT_TRUE(node->modifiers.empty());
        ASSERT_EQ(node->fields.size(), 2);

        EXPECT_EQ(node->fields[0].name, "x");
        EXPECT_TRUE(node->fields[0].modifiers.empty());

        EXPECT_EQ(node->fields[1].name, "y");
        EXPECT_TRUE(node->fields[1].modifiers.empty());
    }

    TEST_F(AstBaseTest, ValidatesStackedModifiersOnStructAndFields) {
        auto ast = parse_code(
            "@export @packed "
            "struct User { "
            "  @id id: int, "
            "  @json(name: \"username\") @unique name: string "
            "}"
        );

        ASSERT_EQ(ast->struct_definitions.size(), 1);
        auto node = ast->struct_definitions[0].get();

        // 1. Verify Struct-Level Modifiers
        ASSERT_EQ(node->modifiers.size(), 2);
        EXPECT_EQ(node->modifiers[0].name, "export");
        EXPECT_EQ(node->modifiers[1].name, "packed");

        // 2. Verify Field-Level Modifiers
        ASSERT_EQ(node->fields.size(), 2);

        // Field: id
        EXPECT_EQ(node->fields[0].name, "id");
        ASSERT_EQ(node->fields[0].modifiers.size(), 1);
        EXPECT_EQ(node->fields[0].modifiers[0].name, "id");

        // Field: name
        EXPECT_EQ(node->fields[1].name, "name");
        ASSERT_EQ(node->fields[1].modifiers.size(), 2);
        EXPECT_EQ(node->fields[1].modifiers[0].name, "json");
        EXPECT_EQ(node->fields[1].modifiers[1].name, "unique");

        // Check argument on @json
        const auto &json_mod = node->fields[1].modifiers[0];
        ASSERT_EQ(json_mod.arguments.size(), 1);
        EXPECT_EQ(json_mod.arguments[0].first, "name");
        EXPECT_EQ(dynamic_cast<StringLiteral*>(json_mod.arguments[0].second.get())->value, "\"username\"");
    }

    TEST_F(AstBaseTest, ValidatesStructFieldModifierWithComplexArguments) {
        auto ast = parse_code(
            "struct Data { "
            "  @clamp(min: 0, max: 10 * 10) "
            "  @meta(tags: [\"raw\", \"input\"], config: { sync: true }) "
            "  value: int "
            "}"
        );

        ASSERT_EQ(ast->struct_definitions.size(), 1);
        auto node = ast->struct_definitions[0].get();
        const auto &field = node->fields[0];

        ASSERT_EQ(field.modifiers.size(), 2);

        // Mod 0: @clamp(min: 0, max: 10 * 10)
        const auto &clamp = field.modifiers[0];
        EXPECT_EQ(clamp.name, "clamp");
        ASSERT_EQ(clamp.arguments.size(), 2);

        // Arg 1 (Math): max: 10 * 10
        EXPECT_EQ(clamp.arguments[1].first, "max");
        auto math = dynamic_cast<BinaryExpression *>(clamp.arguments[1].second.get());
        ASSERT_NE(math, nullptr);
        EXPECT_EQ(math->op, TokenType::Star);

        // Mod 1: @meta(tags: [...], config: {...})
        const auto &meta = field.modifiers[1];
        EXPECT_EQ(meta.name, "meta");
        ASSERT_EQ(meta.arguments.size(), 2);

        // Arg 0 (Tensor): tags: ["raw", "input"]
        EXPECT_EQ(meta.arguments[0].first, "tags");
        auto tensor = dynamic_cast<TensorLiteral *>(meta.arguments[0].second.get());
        ASSERT_NE(tensor, nullptr);
        EXPECT_EQ(tensor->elements.size(), 2);

        // Arg 1 (Dict): config: { sync: true }
        EXPECT_EQ(meta.arguments[1].first, "config");
        auto dict = dynamic_cast<DictLiteral *>(meta.arguments[1].second.get());
        ASSERT_NE(dict, nullptr);
        EXPECT_EQ(dict->elements[0].key, "sync");
        EXPECT_EQ(dynamic_cast<BooleanLiteral*>(dict->elements[0].value.get())->value, true);
    }

    TEST_F(AstBaseTest, ValidatesStructFieldModifierWithEmptyParens) {
        auto ast = parse_code(
            "struct Settings { "
            "  @internal() @obsolete(reason: \"old\") "
            "  flag: bool, "
            "}"
        );

        ASSERT_EQ(ast->struct_definitions.size(), 1);
        auto node = ast->struct_definitions[0].get();
        const auto &field = node->fields[0];

        ASSERT_EQ(field.modifiers.size(), 2);

        // @internal() -> 0 args
        EXPECT_EQ(field.modifiers[0].name, "internal");
        EXPECT_TRUE(field.modifiers[0].arguments.empty());

        // @obsolete(reason: "old",) -> 1 arg
        EXPECT_EQ(field.modifiers[1].name, "obsolete");
        ASSERT_EQ(field.modifiers[1].arguments.size(), 1);
        EXPECT_EQ(field.modifiers[1].arguments[0].first, "reason");
    }

    TEST_F(AstBaseTest, ValidatesStructFieldModifiersWithDotAccess) {
        auto ast = parse_code(
            "struct Window { "
            "  @theme(color: Colors.Blue, font: Fonts.Main.Bold) "
            "  title: string "
            "}"
        );

        ASSERT_EQ(ast->struct_definitions.size(), 1);
        const auto &field = ast->struct_definitions[0]->fields[0];
        const auto &mod = field.modifiers[0];

        ASSERT_EQ(mod.arguments.size(), 2);

        // color: Colors.Blue
        EXPECT_EQ(mod.arguments[0].first, "color");
        auto dot1 = dynamic_cast<DotAccess *>(mod.arguments[0].second.get());
        ASSERT_NE(dot1, nullptr);
        EXPECT_EQ(dot1->property_name, "Blue");

        // font: Fonts.Main.Bold (Deep Dot Access)
        EXPECT_EQ(mod.arguments[1].first, "font");
        auto dot_deep = dynamic_cast<DotAccess *>(mod.arguments[1].second.get());
        ASSERT_NE(dot_deep, nullptr);
        EXPECT_EQ(dot_deep->property_name, "Bold");

        auto dot_mid = dynamic_cast<DotAccess *>(dot_deep->target.get());
        EXPECT_EQ(dot_mid->property_name, "Main");
    }
}
