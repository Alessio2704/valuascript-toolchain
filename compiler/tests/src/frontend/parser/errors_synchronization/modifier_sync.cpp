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
                .test_name = "multiple_modifiers_garbage_between",
                .source_code = "let @first & @second a = 1\nlet recovery = 1\n",
                .expected_errors = { {.code = LexerErrorCode::InvalidCharacter, .line = 1, .column = 12} },
                .verify_ast = ExpectModifierSet({{.name = "first", .args = {}}, {.name = "second", .args = {}}})
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
            }
        ),
        TestNameGenerator{}
    );
}
