#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    namespace {
        const FunctionDefinition *ExpectRecoveredFunction(const Program &ast, const std::string &expected_name) {
            EXPECT_EQ(ast.execution_steps.size(), 1) << "Expected 'let a = 1' to survive.";

            if (ast.function_definitions.empty()) return nullptr;

            auto it = std::find_if(ast.function_definitions.begin(), ast.function_definitions.end(),
                                   [&](const auto &f) { return f->name == expected_name; });

            if (it == ast.function_definitions.end()) return nullptr;
            return it->get();
        }

        void ExpectFunctionSignature(const FunctionDefinition *func,
                                     const std::vector<std::pair<std::string, std::optional<std::string> > > &
                                     expected_params,
                                     const std::vector<std::string> &expected_returns) {
            ASSERT_NE(func, nullptr) << "Function definition was null!";

            ASSERT_EQ(func->parameters.size(), expected_params.size()) << "Parameter count mismatch!";
            for (size_t i = 0; i < expected_params.size(); ++i) {
                EXPECT_EQ(func->parameters[i].name, expected_params[i].first) << "Param name mismatch at index " << i;
                if (expected_params[i].second.has_value()) {
                    ASSERT_NE(func->parameters[i].type, nullptr);
                    EXPECT_EQ(func->parameters[i].type->name,
                              expected_params[i].second) << "Param type mismatch at index "
         <<
                                 i;
                } else {
                    EXPECT_EQ(func->parameters[i].type.get(), nullptr);
                }
            }

            ASSERT_EQ(func->return_types.size(), expected_returns.size()) << "Return type count mismatch!";
            for (size_t i = 0; i < expected_returns.size(); ++i) {
                ASSERT_NE(func->return_types[i], nullptr);
                EXPECT_EQ(func->return_types[i]->name, expected_returns[i]) << "Return type mismatch at index " << i;
            }
        }

        auto ExpectNoFunctions() {
            return [](const Program &ast) {
                ASSERT_EQ(ast.function_definitions.size(), 0);
                ASSERT_EQ(ast.execution_steps.size(), 1);
            };
        }

        auto ExpectFunction(std::string name,
                            std::vector<std::pair<std::string, std::optional<std::string> > > params = {},
                            std::vector<std::string> returns = {"void"}) {
            return [name = std::move(name), params = std::move(params), returns = std::move(returns)](
                const Program &ast) {
                auto f = ExpectRecoveredFunction(ast, name);
                ExpectFunctionSignature(f, params, returns);
            };
        }
    }

    class FunctionParametersAndReturnTypeParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(FunctionParametersAndReturnTypeParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        FunctionParametersAndReturnTypeStressTest,
        FunctionParametersAndReturnTypeParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "no_name_func_empty_ast",
            "func (a: int) -> int {}\n"
            "let a = 1\n",
            {{Err::MissingFunctionName, 1, 6}},
            ExpectFunction("<error>", {{"a", "int"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_name_func_full_ast",
            "func true(a: int) -> int {}\n"
            "let a = 1\n",
            {{Err::ReservedKeywordAsIdentifier, 1, 6}},
            ExpectFunction("true", {{"a", "int"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "no_left_paren_func_empty_ast",
            "func test a: int) -> int {}\n"
            "let a = 1\n",
            {{Err::ExpectedLeftParenAfterFunctionName, 1, 11}},
            ExpectNoFunctions()
            },
            ParserErrorsSynchronizationTestCase{
            "missing_right_paren_1_func_empty_ast",
            "func test(a: int -> int {}\n"
            "let a = 1\n",
            {{Err::ExpectedRightParenAfterParameters, 1, 18}},
            ExpectNoFunctions()
            },
            ParserErrorsSynchronizationTestCase{
            "missing_right_paren_2_func_empty_ast",
            "func test(a: int { return 1 }\n"
            "let a = 1\n",
            {{Err::ExpectedRightParenAfterParameters, 1, 18}},
            ExpectNoFunctions()
            },
            ParserErrorsSynchronizationTestCase{
            "missing_arrow_func",
            "func test(a: int) { return 1 }\n"
            "let a = 1\n",
            {{Err::MissingArrowInFunction, 1, 19}},
            ExpectFunction("test", {{"a", "int"}}, {})
            },
            ParserErrorsSynchronizationTestCase{
            "missing_left_brace_func_empty_ast",
            "func test(a: int) -> int  return 1 }\n"
            "let a = 1\n",
            {{Err::ExpectedLeftBraceBeforeFunctionBody, 1, 27}},
            ExpectNoFunctions()
            },
            ParserErrorsSynchronizationTestCase{
            "missing_right_brace_func",
            "func test(a: int) -> int  { return 1 \n",
            {{Err::ExpectedRightBraceAfterFunctionBody, 1, 37}},
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            ASSERT_EQ(ast.function_definitions[0]->body.size(), 1);
            ASSERT_EQ(ast.execution_steps.size(), 0);
            },
            },
            ParserErrorsSynchronizationTestCase{
            "missing_right_brace_but_valid_body_after",
            "func test(a: int) -> int  { return 1 \n"
            "let a = 1\n",
            {{Err::ExpectedRightBraceAfterFunctionBody, 2, 10}},
            [](const Program& ast) {
            EXPECT_EQ(ast.function_definitions.size(), 1);
            EXPECT_EQ(ast.function_definitions[0]->body.size(), 2);
            EXPECT_EQ(ast.execution_steps.size(), 0);
            },
            },
            ParserErrorsSynchronizationTestCase{
            "missing_comma_in_params_recovers_all",
            "func test(a: int b: string) -> int {}\n"
            "let a = 1\n",
            {{Err::ExpectedCommaSeparatorInParameterList, 1, 18}},
            ExpectFunction("test", {{"a", "int"}, {"b", "string"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "missing_colon_in_params_discards_param_and_recovers",
            "func test(a int, b: string) -> int {}\n"
            "let a = 1\n",
            {{Err::MissingColonAfterParameter, 1, 13}},
            ExpectFunction("test", {{"b", "string"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "garbage_in_params_discards_and_recovers",
            "func test(a: int, *^, b: string) -> int {}\n"
            "let a = 1\n",
            {{Err::MissingParameterName, 1, 19}},
            ExpectFunction("test", {{"a", "int"}, {"<error>", std::nullopt}, {"b", "string"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "multiple_return_types_missing_comma_recovers",
            "func test() -> int string {}\n"
            "let a = 1\n",
            {{Err::ExpectedCommaSeparatorInReturnTypeList, 1, 20}},
            ExpectFunction("test", {}, {"int", "string"})
            },
            ParserErrorsSynchronizationTestCase{
            "missing_type_after_arrow_discards_and_continues",
            "func test() -> , int {}\n"
            "let a = 1\n",
            {{Err::MissingTypeAnnotation, 1, 16}},
            ExpectFunction("test", {}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "error_in_params_preserves_docstring",
            "func test(a: ) -> int { \"\"\"docs\"\"\" }\n"
            "let a = 1\n",
            {{Err::MissingTypeAnnotation, 1, 14}},
            [](const Program &ast) {
            auto f = ExpectRecoveredFunction(ast, "test");
            ASSERT_NE(f, nullptr);
            ASSERT_EQ(f->parameters.size(), 1);
            ASSERT_EQ(f->parameters[0].name, "a");
            ASSERT_EQ(f->parameters[0].type.get(), nullptr);
            ASSERT_EQ(f->return_types.size(), 1);
            ASSERT_EQ(f->return_types[0].get()->name, "int");
            EXPECT_TRUE(f->docstring.has_value());
            EXPECT_EQ(f->docstring.value(), "\"\"\"docs\"\"\"");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "error_in_params_types_1",
            "func test(a: vector<int) -> int {  }\n"
            "let a = 1\n",
            {{Err::UnmatchedBracketAfterGenericArgs, 1, 24}},
            ExpectFunction("test", {{"a", "vector"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "error_in_params_types_2",
            "func test(a: vector<int>, b: vector<decimal, >) -> int {  }\n"
            "let a = 1\n",
            {{Err::TrailingCommaInGenericArgument, 1, 44}},
            ExpectFunction("test", {{"a", "vector"}, {"b", "vector"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "error_in_params_types_3",
            "func test(a: vector<int>, b: vector<>) -> int {  }\n"
            "let a = 1\n",
            {{Err::EmptyGenericTypeAnnotation, 1, 37}},
            ExpectFunction("test", {{"a", "vector"}, {"b", "vector"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "missing_colon",
            "func test(a, b, c) -> int { return 1 }\n"
            "let a = 1\n",
            {
            {Err::MissingColonAfterParameter, 1, 12},
            {Err::MissingColonAfterParameter, 1, 15},
            {Err::MissingColonAfterParameter, 1, 18},
            },
            ExpectFunction("test", {{"a", std::nullopt}, {"b", std::nullopt}, {"c", std::nullopt}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "missing_type_annotation_arguments_1",
            "func test(a: , b: , c: ) -> int { return 1 }\n"
            "let a = 1\n",
            {
            {Err::MissingTypeAnnotation, 1, 14},
            {Err::MissingTypeAnnotation, 1, 19},
            {Err::MissingTypeAnnotation, 1, 24},
            },
            ExpectFunction("test", {{"a", std::nullopt}, {"b", std::nullopt}, {"c", std::nullopt}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "missing_type_annotation_arguments_2",
            "func test(a: int, b: , c: string d: decimal) -> int { return 1 }\n"
            "let a = 1\n",
            {
            {Err::MissingTypeAnnotation, 1, 22},
            {Err::ExpectedCommaSeparatorInParameterList, 1, 34},
            },
            ExpectFunction("test", {{"a", "int"}, {"b", std::nullopt}, {"c", "string"}, {"d", "decimal"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "error_inside_return_tuple_recovers",
            "func test() -> (int, ) {}\n"
            "let a = 1\n",
            { {Err::SingleElementTuplesNotAllowed, 1, 20} },
            ExpectFunction("test", {}, {"tuple"})
            },
            ParserErrorsSynchronizationTestCase{
            "first_func_error_does_not_break_second_func",
            "func first(a: ) -> void {}\n"
            "func second(b: int) -> void {}\n"
            "let a = 1\n",
            { {Err::MissingTypeAnnotation, 1, 15} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 2);
            EXPECT_EQ(ast.function_definitions[0]->name, "first");
            EXPECT_EQ(ast.function_definitions[1]->name, "second");
            EXPECT_EQ(ast.execution_steps.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_params_1",
            "func test(let: int) -> int {}\n"
            "let a = 1\n",
            {{Err::ReservedKeywordAsIdentifier, 1, 11}},
            ExpectFunction("test", {{"let", "int"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_params_2",
            "func test(let: int, if: int, then: string) -> int {}\n"
            "let a = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 11},
            {Err::ReservedKeywordAsIdentifier, 1, 21},
            {Err::ReservedKeywordAsIdentifier, 1, 30},
            },
            ExpectFunction("test", {{"let", "int"}, {"if", "int"}, {"then", "string"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_params_3",
            "func test(let: int if: int then: string) -> int {}\n"
            "let a = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 11},
            {Err::ExpectedCommaSeparatorInParameterList, 1, 20},
            {Err::ReservedKeywordAsIdentifier, 1, 20},
            {Err::ExpectedCommaSeparatorInParameterList, 1, 28},
            {Err::ReservedKeywordAsIdentifier, 1, 28},
            },
            ExpectFunction("test", {{"let", "int"}, {"if", "int"}, {"then", "string"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_params_type_1",
            "func test(a: true) -> int {}\n"
            "let a = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 14},
            },
            ExpectFunction("test", {{"a", "true"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "return_type_1",
            "func test(a: int) -> true {}\n"
            "let a = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 22},
            },
            ExpectFunction("test", {{"a", "int"}}, {"true"})
            },
            ParserErrorsSynchronizationTestCase{
            "return_type_2",
            "func test(a: int) -> vector<true> {}\n"
            "let a = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 29},
            },
            ExpectFunction("test", {{"a", "int"}}, {"vector"})
            },
            ParserErrorsSynchronizationTestCase{
            "return_type_3",
            "func test(a: int) -> (int int) {}\n"
            "let a = 1\n",
            {
            {Err::ExpectedCommaSeparatorInTupleType, 1, 27},
            },
            ExpectFunction("test", {{"a", "int"}}, {"tuple"})
            },
            ParserErrorsSynchronizationTestCase{
            "missing_default_parameter_value_syncs_to_comma",
            "func test(a: int =, b: string) -> int {}\n"
            "let a = 1\n",
            {
            {Err::MissingDefaultParameterValue, 1, 18},
            {Err::NonDefaultParameterAfterDefault, 1, 21},
            },
            ExpectFunction("test", {{"a", "int"}, {"b", "string"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "missing_default_parameter_value_syncs_to_paren",
            "func test(a: int =) -> int {}\n"
            "let a = 1\n",
            {{Err::MissingDefaultParameterValue, 1, 18}},
            [](const Program &ast) {
            auto f = ExpectRecoveredFunction(ast, "test");
            ASSERT_NE(f, nullptr);
            ASSERT_EQ(f->parameters.size(), 1);
            EXPECT_EQ(f->parameters[0].name, "a");
            auto type = dynamic_cast<TypeAnnotation*>(f->parameters[0].type.get());
            EXPECT_EQ(type->name, "int");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "non_default_parameter_after_default_reports_error",
            "func test(a: int = 1, b: int) -> int {}\n"
            "let a = 1\n",
            {{Err::NonDefaultParameterAfterDefault, 1, 23}},
            [](const Program &ast) {
            auto f = ExpectRecoveredFunction(ast, "test");
            ASSERT_NE(f, nullptr);
            ASSERT_EQ(f->parameters.size(), 2);
            EXPECT_EQ(f->parameters[0].name, "a");
            EXPECT_NE(f->parameters[0].default_value, nullptr);
            EXPECT_EQ(f->parameters[1].name, "b");
            EXPECT_EQ(f->parameters[1].default_value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "invalid_expression_in_default_value_recovers_to_comma",
            "func test(a: int = *, b: string) -> int {}\n"
            "let a = 1\n",
            {
            {Err::InvalidExpression, 1, 20},
            {Err::NonDefaultParameterAfterDefault, 1, 23},
            },
            ExpectFunction("test", {{"a", "int"}, {"b", "string"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "multiple_non_default_parameters_after_default_reports_multiple_errors",
            "func test(a: int = 1, b: int, c: int) -> int {}\n"
            "let a = 1\n",
            {
            {Err::NonDefaultParameterAfterDefault, 1, 23},
            {Err::NonDefaultParameterAfterDefault, 1, 31}
            },
            [](const Program &ast) {
            auto f = ExpectRecoveredFunction(ast, "test");
            ASSERT_NE(f, nullptr);
            ASSERT_EQ(f->parameters.size(), 3);
            EXPECT_NE(f->parameters[0].default_value, nullptr);
            EXPECT_EQ(f->parameters[1].default_value, nullptr);
            EXPECT_EQ(f->parameters[2].default_value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "valid_default_parameters_parse_correctly",
            "func test(a: int = 1, b: string = \"hello\") -> int {}\n"
            "let a = 1\n",
            {},
            [](const Program &ast) {
            auto f = ExpectRecoveredFunction(ast, "test");
            ASSERT_NE(f, nullptr);
            ASSERT_EQ(f->parameters.size(), 2);
            EXPECT_NE(f->parameters[0].default_value, nullptr);
            EXPECT_NE(f->parameters[1].default_value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "valid_dict_literal_default_value",
            "func test(a: dict = {key: 1}, b: int = 2) -> int {}\n"
            "let a = 1\n",
            {},
            [](const Program &ast) {
            auto f = ExpectRecoveredFunction(ast, "test");
            ASSERT_NE(f, nullptr);
            ASSERT_EQ(f->parameters.size(), 2);
            EXPECT_NE(f->parameters[0].default_value, nullptr);
            EXPECT_NE(f->parameters[1].default_value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "valid_tensor_literal_default_value",
            "func test(a: vector = [1, 2, 3]) -> int {}\n"
            "let a = 1\n",
            {},
            [](const Program &ast) {
            auto f = ExpectRecoveredFunction(ast, "test");
            ASSERT_NE(f, nullptr);
            ASSERT_EQ(f->parameters.size(), 1);
            EXPECT_NE(f->parameters[0].default_value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "broken_tuple_default_value_recovers",
            "func test(a: tuple = (1, *), b: int = 1) -> int {}\n"
            "let a = 1\n",
            {{Err::InvalidExpression, 1, 26}},
            [](const Program &ast) {
            auto f = ExpectRecoveredFunction(ast, "test");
            ASSERT_NE(f, nullptr);
            ASSERT_EQ(f->parameters.size(), 2);
            EXPECT_EQ(f->parameters[0].name, "a");
            EXPECT_NE(f->parameters[0].default_value, nullptr);
            EXPECT_EQ(f->parameters[1].name, "b");
            EXPECT_NE(f->parameters[1].default_value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "broken_tensor_default_value_recovers",
            "func test(a: vector = [1, *], b: int = 1) -> int {}\n"
            "let a = 1\n",
            {{Err::InvalidExpression, 1, 27}},
            [](const Program &ast) {
            auto f = ExpectRecoveredFunction(ast, "test");
            ASSERT_NE(f, nullptr);
            ASSERT_EQ(f->parameters.size(), 2);
            EXPECT_EQ(f->parameters[0].name, "a");
            EXPECT_NE(f->parameters[0].default_value, nullptr);
            EXPECT_EQ(f->parameters[1].name, "b");
            EXPECT_NE(f->parameters[1].default_value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "broken_dict_default_value_recovers",
            "func test(a: dict = {x *}, b: int = 1) -> int {}\n"
            "let a = 1\n",
            {{Err::ExpectedColonAfterDictionaryKey, 1, 24}},
            [](const Program &ast) {
            auto f = ExpectRecoveredFunction(ast, "test");
            ASSERT_NE(f, nullptr);
            ASSERT_EQ(f->parameters.size(), 2);
            EXPECT_EQ(f->parameters[0].name, "a");
            EXPECT_NE(f->parameters[0].default_value, nullptr);
            EXPECT_EQ(f->parameters[1].name, "b");
            EXPECT_NE(f->parameters[1].default_value, nullptr);
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase> &info) {
        return info.param.test_name;
        }
    );
}
