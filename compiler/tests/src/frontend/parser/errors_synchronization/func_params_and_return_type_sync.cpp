#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const FunctionDefinition* ExpectRecoveredFunction(const Program& ast, const std::string& expected_name)
        {
            EXPECT_EQ(ast.execution_steps.size(), 1) << "Expected 'let a = 1' to survive.";

            if (ast.function_definitions.empty()) return nullptr;

            auto it = std::find_if(ast.function_definitions.begin(), ast.function_definitions.end(),
                                   [&](const auto& f) { return f->name == expected_name; });

            if (it == ast.function_definitions.end()) return nullptr;
            return it->get();
        }

        void ExpectFunctionSignature(const FunctionDefinition* func,
                                     const std::vector<std::pair<std::string, std::optional<std::string>>>&
                                     expected_params,
                                     const std::vector<std::string>& expected_returns)
        {
            ASSERT_NE(func, nullptr) << "Function definition was null!";

            ASSERT_EQ(func->parameters.size(), expected_params.size()) << "Parameter count mismatch!";
            for (size_t i = 0; i < expected_params.size(); ++i)
            {
                EXPECT_EQ(func->parameters[i].name, expected_params[i].first) << "Param name mismatch at index " << i;
                if (expected_params[i].second.has_value())
                {
                    ASSERT_NE(func->parameters[i].type, nullptr);
                    EXPECT_EQ(func->parameters[i].type->name,
                              expected_params[i].second) << "Param type mismatch at index "
         <<
                                 i;
                }
                else
                {
                    EXPECT_EQ(func->parameters[i].type.get(), nullptr);
                }
            }

            ASSERT_EQ(func->return_types.size(), expected_returns.size()) << "Return type count mismatch!";
            for (size_t i = 0; i < expected_returns.size(); ++i)
            {
                ASSERT_NE(func->return_types[i], nullptr);
                if (expected_returns[i].empty())
                {
                    EXPECT_EQ(func->return_types[i], nullptr);

                }
                else
                {
                    EXPECT_EQ(func->return_types[i]->name, expected_returns[i]) << "Return type mismatch at index " <<
 i;
                }
            }
        }

        auto ExpectNoFunctions()
        {
            return [](const Program& ast)
            {
                ASSERT_EQ(ast.function_definitions.size(), 0);
                ASSERT_EQ(ast.execution_steps.size(), 1);
            };
        }

        auto ExpectFunction(std::string name,
                            std::vector<std::pair<std::string, std::optional<std::string>>> params = {},
                            std::vector<std::string> returns = {"void"})
        {
            return [n = std::move(name), par = std::move(params), ret = std::move(returns)](
                const Program& ast)
            {
                auto f = ExpectRecoveredFunction(ast, n);
                ExpectFunctionSignature(f, par, ret);
            };
        }
    }

    class FunctionParametersAndReturnTypeParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(FunctionParametersAndReturnTypeParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        FunctionParametersAndReturnTypeStressTest,
        FunctionParametersAndReturnTypeParserSynchronizationTest,
        ::testing::Values(
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
            "error_in_params_types_1",
            "func test(a: vector<int) -> int {  }\n"
            "let a = 1\n",
            {{Err::UnmatchedBracketAfterGenericArgs, 1, 24}},
            ExpectFunction("test", {{"a", "vector"}}, {"int"})
            },
            ParserErrorsSynchronizationTestCase{
            "error_in_params_types_3",
            "func test(a: vector<int>, b: vector<>) -> int {  }\n"
            "let a = 1\n",
            {{Err::EmptyGenericTypeAnnotation, 1, 37}},
            ExpectFunction("test", {{"a", "vector"}, {"b", "vector"}}, {"int"})
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase> &test_info) {
        return test_info.param.test_name;
        }
    );
}
