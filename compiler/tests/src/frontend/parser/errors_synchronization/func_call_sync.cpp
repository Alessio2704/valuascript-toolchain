#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const FunctionCall* ExpectRecoveredFunctionCall(const Program& ast, const std::string& expected_target)
        {
            EXPECT_EQ(ast.execution_steps.size(), 2) << "Expected 'f(...)' and 'let recovery = 1' to survive.";
            if (ast.execution_steps.empty()) return nullptr;

            auto* expr_stmt = dynamic_cast<ExpressionStatement*>(ast.execution_steps.front().get());
            EXPECT_NE(expr_stmt, nullptr) << "First execution step is not an ExpressionStatement";
            if (!expr_stmt) return nullptr;

            auto* func_call = dynamic_cast<FunctionCall*>(expr_stmt->expr.get());
            EXPECT_NE(func_call, nullptr) << "Expression is not a FunctionCall";
            if (!func_call) return nullptr;

            auto* target_id = dynamic_cast<IdentifierAccess*>(func_call->target.get());
            EXPECT_NE(target_id, nullptr) << "Target is not an IdentifierAccess";
            if (target_id)
            {
                EXPECT_EQ(target_id->name, expected_target);
            }

            return func_call;
        }

        struct ExpectedArgument
        {
            std::string name;
            std::optional<std::string> expected_number_value;
            std::function<void(const Expression*)> verifier;

            ExpectedArgument(const char* n) : name(n), expected_number_value(std::nullopt)
            {
            }

            ExpectedArgument(const char* n, const char* v) : name(n), expected_number_value(std::string(v))
            {
            }

            ExpectedArgument(const char* n, const std::optional<std::string>& v) : name(n), expected_number_value(v)
            {
            }

            ExpectedArgument(const char* n,
                             std::function<void(const Expression*)> v) : name(n), verifier(std::move(v))
            {
            }
        };

        void ExpectFunctionCallArgs(const FunctionCall* func_call,
                                    const std::vector<ExpectedArgument>& expected_args)
        {
            ASSERT_NE(func_call, nullptr) << "Function call was null!";
            ASSERT_EQ(func_call->arguments.size(), expected_args.size()) << "Recovered argument count mismatch!";

            for (size_t i = 0; i < expected_args.size(); ++i)
            {
                EXPECT_EQ(func_call->arguments[i].first, expected_args[i].name)
                     << "Argument name mismatch at index " << i;

                if (expected_args[i].verifier)
                {
                    expected_args[i].verifier(func_call->arguments[i].second.get());
                }
                else if (expected_args[i].expected_number_value.has_value())
                {
                    ASSERT_NE(func_call->arguments[i].second, nullptr)
                         << "Expected an assigned value for argument '" << expected_args[i].name << "' but got nullptr";
                    auto* num_lit = dynamic_cast<NumberLiteral*>(func_call->arguments[i].second.get());
                    ASSERT_NE(num_lit, nullptr)
                         << "Expected a NumberLiteral for argument '" << expected_args[i].name << "'";

                    EXPECT_EQ(num_lit->value, expected_args[i].expected_number_value.value())
                         << "Assigned value mismatch for argument '" << expected_args[i].name << "'";
                }
                else
                {
                    EXPECT_EQ(func_call->arguments[i].second, nullptr)
                         << "Expected NO assigned value for argument '" << expected_args[i].name << "' but found one";
                }
            }
        }

        auto ExpectFunctionCall(std::string target_name, std::vector<ExpectedArgument> args = {})
        {
            return [t = std::move(target_name), a = std::move(args)](const Program& ast)
            {
                auto call = ExpectRecoveredFunctionCall(ast, t);
                ExpectFunctionCallArgs(call, a);
            };
        }
    }

    class FunctionCallParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(FunctionCallParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        FunctionCallStressTests,
        FunctionCallParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "function_call_in_assignment_missing_closing_paren",
            "let a = f(a: 1\n"
            "let recovery = 1\n",
            {
            {Err::ExpectedRightParenAfterArguments, 1, 14},
            },
            [](const Program &ast) {
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            auto func_call = dynamic_cast<FunctionCall*>(assign->value.get());
            ASSERT_NE(func_call, nullptr);
            EXPECT_EQ(func_call->arguments.size(), 1);
            auto func_call_id = dynamic_cast<IdentifierAccess*>(func_call->target.get());
            ASSERT_NE(func_call_id, nullptr);
            EXPECT_EQ(func_call_id->name, "f");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "missing_closing_parenthesis",
            "f(a: 1 \n"
            "let recovery = 1\n",
            {
            {Err::ExpectedRightParenAfterArguments, 1, 6}
            },
            ExpectFunctionCall("f", {{"a", "1"}})
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_as_value",
            "f(a: 1, b: struct, c: 3)\n"
            "let recovery = 1\n",
            { {Err::ReservedKeywordAsIdentifier, 1, 12} },
            ExpectFunctionCall("f", {{"a", "1"}, {"b", [](const Expression* e) {
                auto const identifier = dynamic_cast<const IdentifierAccess*>(e);
                ASSERT_NE(identifier, nullptr);
                ASSERT_EQ(identifier->name, "struct");
                }}, {"c", "3"}})
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
            return test_info.param.test_name;
        }
    );
}
