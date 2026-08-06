#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const std::vector<Modifier>* GetModifiersFromFirstStmt(const Program& ast)
        {
            if (ast.execution_steps.empty()) return nullptr;

            auto* assign = dynamic_cast<Assignment*>(ast.execution_steps.front().get());
            if (!assign || assign->targets.empty()) return nullptr;

            return &assign->targets.front().modifiers;
        }

        struct ExpectedArgument
        {
            std::string name;
            std::optional<std::string> expected_number_value = std::nullopt;
            std::function<void(const Expression*)> verifier = nullptr;
        };

        struct ExpectedModifier
        {
            std::string name;
            std::vector<ExpectedArgument> args = {};
        };

        void ExpectModifiers(const Program& ast, const std::vector<ExpectedModifier>& expected)
        {
            auto* actual_mods = GetModifiersFromFirstStmt(ast);
            ASSERT_NE(actual_mods, nullptr) << "First statement is not an Assignment or has no targets field.";

            ASSERT_EQ(actual_mods->size(), expected.size()) << "Modifier count mismatch!";

            for (size_t i = 0; i < expected.size(); ++i)
            {
                EXPECT_EQ((*actual_mods)[i].name, expected[i].name) << "Modifier name mismatch at index " << i;

                const auto& actual_args = (*actual_mods)[i].arguments;
                const auto& expected_args = expected[i].args;

                ASSERT_EQ(actual_args.size(), expected_args.size())
                     << "Argument count mismatch for modifier @" << expected[i].name;

                for (size_t j = 0; j < expected_args.size(); ++j)
                {
                    EXPECT_EQ(actual_args[j].first, expected_args[j].name)
                         << "Arg name mismatch in @" << expected[i].name;

                    if (expected_args[j].expected_number_value.has_value())
                    {
                        auto* num = dynamic_cast<NumberLiteral*>(actual_args[j].second.get());
                        ASSERT_NE(num, nullptr) << "Expected number literal in @" << expected[i].name;
                        EXPECT_EQ(num->value, *expected_args[j].expected_number_value);
                    }
                }
            }
        }

        auto ExpectModifierSet(std::vector<ExpectedModifier> mods)
        {
            return [m = std::move(mods)](const Program& ast)
            {
                ExpectModifiers(ast, m);
                EXPECT_GT(ast.execution_steps.size(), 1);
            };
        }
    }

    class ModifierParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(ModifierParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        ModifierStressTests,
        ModifierParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "modifier_missing_closing_paren",
                .source_code = "let @test(a: 1\na = 1let recovery = 1\n",
                .expected_errors = { {.code = Err::UnmatchedParenthesisAfterModifierArgs, .line = 1, .column = 14} },
                .verify_ast = ExpectModifierSet({{.name = "test", .args = {{.name = "a"}}}})
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "multiple_modifiers_garbage_between",
                .source_code = "let @first & @second a = 1\nlet recovery = 1\n",
                .expected_errors = { {.code = LexerErrorCode::InvalidCharacter, .line = 1, .column = 12} },
                .verify_ast = ExpectModifierSet({{.name = "first", .args = {}}, {.name = "second", .args = {}}})
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "modifier_unmatched_arg_paren_at_eof",
                .source_code = "@test(a: 1\n",
                .expected_errors = {
                    {.code = Err::UnmatchedParenthesisAfterModifierArgs, .line = 1, .column = 10},
                    {.code = Err::ModifiersAttachedToInvalidDeclaration, .line = 1, .column = 1},
                },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 0);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "modifier_eof_after_at",
                .source_code = "let a = 1\n@",
                .expected_errors = {
                    {.code = Err::ExpectedModifierName, .line = 2, .column = 2},
                    {.code = Err::ModifiersAttachedToInvalidDeclaration, .line = 2, .column = 1},
                },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 1);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "modifier_eof_after_paren",
                .source_code = "@test(",
                .expected_errors = {
                    {.code = Err::UnmatchedParenthesisAfterModifierArgs, .line = 1, .column = 6},
                    {.code = Err::ModifiersAttachedToInvalidDeclaration, .line = 1, .column = 1},
                },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 0);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "dict_key_modifier_unmatched_paren",
                .source_code = "let obj = { @test(a: 1 key: 1 }\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::MissingCommaSeparatorForArgumentsInModifier, .line = 1, .column = 24},
                    {.code = Err::UnmatchedParenthesisAfterModifierArgs, .line = 1, .column = 29}
                },
                .verify_ast = [](const Program& ast) {
                    auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
                    EXPECT_EQ(dict->elements.size(), 1);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "func_param_modifier_missing_right_paren_in_args",
                .source_code = "func f(@test(a: 1 p: int) -> void {}\nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::MissingCommaSeparatorForArgumentsInModifier, .line = 1, .column = 19},
                    {.code = Err::MissingParameterName, .line = 1, .column = 27},
                    {.code = Err::ExpectedRightParenAfterParameters, .line = 1, .column = 36}
                },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.function_definitions.size(), 0);

                    ASSERT_EQ(ast.execution_steps.size(), 1);
                    auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    ASSERT_NE(assign, nullptr);
                    EXPECT_EQ(assign->targets[0].name, "recovery");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "struct_field_modifier_missing_right_brace_in_struct",
                .source_code = "struct S { @test(a: 1) id: int \nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::ExpectedRightBraceAfterStructBody, .line = 1, .column = 30}
                },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.struct_definitions.size(), 1);
                    EXPECT_EQ(ast.struct_definitions[0]->name, "S");

                    ASSERT_EQ(ast.execution_steps.size(), 1);
                    auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    ASSERT_NE(assign, nullptr);
                    EXPECT_EQ(assign->targets[0].name, "recovery");
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "struct_field_modifier_unmatched_paren_stretches_to_eof",
                .source_code = "struct S { @test(a: 1 \n",
                .expected_errors = {
                    {.code = Err::UnmatchedParenthesisAfterModifierArgs, .line = 1, .column = 21},
                    {.code = Err::ExpectedStructFieldName, .line = 1, .column = 22},
                    {.code = Err::ExpectedRightBraceAfterStructBody, .line = 1, .column = 21}
                },
                .verify_ast = [](const Program& ast) {
                    EXPECT_EQ(ast.execution_steps.size(), 0);
                }
            }
        ),
        TestNameGenerator{}
    );
}
