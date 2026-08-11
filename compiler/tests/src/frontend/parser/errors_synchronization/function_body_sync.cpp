#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const FunctionDefinition* ExpectRecoveredFunction(const Program& ast, const std::string& expected_name)
        {
            EXPECT_EQ(ast.execution_steps.size(), 1) << "Expected 'let a = 1' to survive at the top level.";

            if (ast.function_definitions.empty()) return nullptr;

            auto it = std::find_if(ast.function_definitions.begin(), ast.function_definitions.end(),
                                   [&](const auto& f) { return f->name == expected_name; });

            if (it == ast.function_definitions.end()) return nullptr;
            return it->get();
        }

        auto ExpectFunctionBodySize(std::string name, size_t expected_body_statements)
        {
            return [n = std::move(name), expected_body_statements](const Program& ast)
            {
                auto f = ExpectRecoveredFunction(ast, n);
                ASSERT_NE(f, nullptr) << "Function not found!";
                EXPECT_EQ(f->body.size(), expected_body_statements) << "Function body statement count mismatch!";
            };
        }
    }

    class FunctionBodyParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(FunctionBodyParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        FunctionBodyStressTests,
        FunctionBodyParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "missing_closing_brace_escapes_to_top_level",
                .source_code = "func test() -> int {\n    let x = 1\nlet a = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightBraceAfterFunctionBody, .line = 3, .column = 9} },
                .verify_ast = [](const Program &ast) {
                    ASSERT_EQ(ast.function_definitions.size(), 1);
                    ASSERT_EQ(ast.function_definitions[0]->body.size(), 2);
                    EXPECT_EQ(ast.execution_steps.size(), 0);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "dangling_modifier_at_end_of_block",
                .source_code = "func test() -> int {\n    let x = 1\n    @\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::ExpectedModifierName, .line = 3, .column = 6},
                    {.code = Err::ModifiersAttachedToInvalidDeclaration, .line = 3, .column = 5}
                },
                .verify_ast = ExpectFunctionBodySize("test", 1)
            }
        ),
        TestNameGenerator{}
    );
}
