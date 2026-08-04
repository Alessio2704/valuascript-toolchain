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
                .test_name = "missing_brace_recovers_at_enum",
                .source_code = "func test() -> int {\n    let x = 1\nenum State: int { A = 1 }\n",
                .expected_errors = { {.code = Err::ExpectedRightBraceAfterFunctionBody, .line = 2, .column = 13} },
                .verify_ast = [](const Program &ast) {
                    ASSERT_EQ(ast.function_definitions.size(), 1);
                    EXPECT_EQ(ast.function_definitions[0]->name, "test");
                    EXPECT_EQ(ast.function_definitions[0]->body.size(), 1);

                    ASSERT_EQ(ast.enum_definitions.size(), 1);
                    EXPECT_EQ(ast.enum_definitions[0]->name, "State");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "missing_brace_recovers_at_struct_with_modifiers",
                .source_code = "func test() -> int {\n    let x = 1\n@export @packed(align: 4) struct Data { id: int }\n",
                .expected_errors = { {.code = Err::ExpectedRightBraceAfterFunctionBody, .line = 2, .column = 13} },
                .verify_ast = [](const Program &ast) {
                    ASSERT_EQ(ast.function_definitions.size(), 1);
                    EXPECT_EQ(ast.function_definitions[0]->body.size(), 1);

                    ASSERT_EQ(ast.struct_definitions.size(), 1);
                    EXPECT_EQ(ast.struct_definitions[0]->name, "Data");
                    EXPECT_EQ(ast.struct_definitions[0]->fields.size(), 1);
                    EXPECT_EQ(ast.struct_definitions[0]->modifiers.size(), 2);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "missing_brace_recovers_at_new_function",
                .source_code = "func first() -> int {\n    let x = 1\n@test @test_1 @test_2(a: 1) func second() -> void {}\n",
                .expected_errors = { {.code = Err::ExpectedRightBraceAfterFunctionBody, .line = 2, .column = 13} },
                .verify_ast = [](const Program &ast) {
                    ASSERT_EQ(ast.function_definitions.size(), 2);

                    EXPECT_EQ(ast.function_definitions[0]->name, "first");
                    EXPECT_EQ(ast.function_definitions[0]->body.size(), 1);

                    EXPECT_EQ(ast.function_definitions[1]->name, "second");
                    EXPECT_EQ(ast.function_definitions[1]->modifiers.size(), 3);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "missing_brace_recovers_at_directive",
                .source_code = "func test() -> int {\n    let x = 1\n#pragma = 1\n",
                .expected_errors = { {.code = Err::ExpectedRightBraceAfterFunctionBody, .line = 2, .column = 13} },
                .verify_ast = [](const Program &ast) {
                    ASSERT_EQ(ast.function_definitions.size(), 1);
                    EXPECT_EQ(ast.function_definitions[0]->body.size(), 1);

                    ASSERT_EQ(ast.directives.size(), 1);
                    EXPECT_EQ(ast.directives[0]->name, "pragma");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "missing_brace_recovers_at_import",
                .source_code = "func test() -> int {\n    let x = 1\nimport \"module.vs\"\n",
                .expected_errors = { {.code = Err::ExpectedRightBraceAfterFunctionBody, .line = 2, .column = 13} },
                .verify_ast = [](const Program &ast) {
                    ASSERT_EQ(ast.function_definitions.size(), 1);
                    EXPECT_EQ(ast.function_definitions[0]->body.size(), 1);

                    ASSERT_EQ(ast.import_statements.size(), 1);
                    EXPECT_EQ(ast.import_statements[0]->path, "\"module.vs\"");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "broken_function_call_recovers_to_next_statement",
                .source_code = "func test() -> int {\n    foo(a: 1 b: 2)\n    return 1\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::MissingCommaSeparatorForArgumentsInFunctionCall, .line = 2, .column = 14}
                },
                .verify_ast = ExpectFunctionBodySize("test", 2)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "dangling_modifier_at_end_of_block",
                .source_code = "func test() -> int {\n    let x = 1\n    @\n}\nlet a = 1\n",
                .expected_errors = {
                    {.code = Err::ExpectedModifierName, .line = 3, .column = 6},
                    {.code = Err::ModifiersAttachedToInvalidDeclaration, .line = 3, .column = 5}
                },
                .verify_ast = ExpectFunctionBodySize("test", 1)
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "nested_struct_without_closing_brace_escapes_to_top_level",
                .source_code = "func test() -> int {\n    let a = 1\n    struct TopLevel { id: int }\nlet c = 1\n",
                .expected_errors = {
                    {.code = Err::ExpectedRightBraceAfterFunctionBody, .line = 2, .column = 13}
                },
                .verify_ast = [](const Program &ast) {
                    auto f = ExpectRecoveredFunction(ast, "test");
                    ASSERT_NE(f, nullptr);
                    EXPECT_EQ(f->body.size(), 1);

                    ASSERT_EQ(ast.struct_definitions.size(), 1);
                    EXPECT_EQ(ast.struct_definitions[0]->name, "TopLevel");
                }
            }
        ),
        TestNameGenerator{}
    );
}
