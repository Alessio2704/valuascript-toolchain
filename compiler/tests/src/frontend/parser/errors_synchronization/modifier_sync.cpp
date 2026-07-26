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

        struct ExpectedModifier
        {
            std::string name;
            std::vector<ExpectedArgument> args;
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
            "modifier_missing_closing_paren",
            "let @test(a: 1\n"
            "a = 1"
            "let recovery = 1\n",
            { {Err::UnmatchedParenthesisAfterModifierArgs, 1, 15} },
            ExpectModifierSet({{"test", {"a"}}})
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_on_invalid_statement",
            "@test\n"
            "f()\n"
            "let recovery = 1\n",
            { {Err::ModifiersAttachedToInvalidDeclaration, 1, 1} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto* f_call = dynamic_cast<ExpressionStatement*>(ast.execution_steps[0].get());
            ASSERT_NE(f_call, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_on_broken_func",
            "@rpc func f(a:) -> int { return 1 }\n"
            "let recovery = 1\n",
            { {Err::MissingTypeAnnotation, 1, 15} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            ASSERT_EQ(ast.function_definitions[0]->modifiers.size(), 1);
            ASSERT_EQ(ast.function_definitions[0]->modifiers[0].name, "rpc");
            ASSERT_EQ(ast.function_definitions[0]->parameters.size(), 1);
            EXPECT_EQ(ast.execution_steps.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "multiple_modifiers_garbage_between",
            "let @first & @second a = 1\n"
            "let recovery = 1\n",
            { {LexerErrorCode::InvalidCharacter, 1, 12} },
            ExpectModifierSet({ {"first", {}}, {"second", {}}})
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_unmatched_arg_paren_at_eof",
            "@test(a: 1\n",
            {
            {Err::UnmatchedParenthesisAfterModifierArgs, 1, 11},
            {Err::ModifiersAttachedToInvalidDeclaration, 1, 1},
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_on_directive",
            "@test #version = 1\n"
            "let recovery = 1\n",
            { {Err::ModifiersAttachedToInvalidDeclaration, 1, 1} },
            [](const Program& ast) {
            ASSERT_EQ(ast.directives.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_on_reassignment",
            "let x = 0\n"
            "@test x = 1\n"
            "let recovery = 1\n",
            { {Err::ModifiersAttachedToInvalidDeclaration, 2, 1} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 3);
            auto statement = dynamic_cast<Reassignment*>(ast.execution_steps[1].get());
            ASSERT_NE(statement, nullptr);
            auto target = dynamic_cast<IdentifierAccess*>(statement->target.get());
            ASSERT_NE(target, nullptr);
            EXPECT_EQ(target->name, "x");
            auto value = dynamic_cast<NumberLiteral*>(statement->value.get());
            EXPECT_EQ(value->value, "1");
            ASSERT_NE(value, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_eof_after_at",
            "let a = 1\n@",
            {
            {Err::ExpectedModifierName, 2, 2},
            {Err::ModifiersAttachedToInvalidDeclaration, 2, 1},
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_eof_after_paren",
            "@test(",
            {
            {Err::UnmatchedParenthesisAfterModifierArgs, 1, 7},
            {Err::ModifiersAttachedToInvalidDeclaration, 1, 1},
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 0);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_interrupted_by_hash_directive",
            "@test \n #directive let a = 1\n"
            "let recovery = 1\n",
            { {Err::ModifiersAttachedToInvalidDeclaration, 1, 1} },
            [](const Program& ast) {
            ASSERT_EQ(ast.directives.size(), 1);
            ASSERT_EQ(ast.execution_steps.size(), 2);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_modifier_missing_name_and_dict_key",
            "let obj = { @ 1: 1, other: 2 }\nlet recovery = 1\n",
            { {Err::ExpectedModifierName, 1, 15},
            {Err::ExpectedDictionaryKey, 1, 16}
            },
            [](const Program& ast) {
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_EQ(dict->elements.size(), 2);
            EXPECT_EQ(dict->elements[0].key, "<error>");
            EXPECT_EQ(dict->elements[1].key, "other");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_key_modifier_unmatched_paren",
            "let obj = { @test(a: 1 key: 1 }\nlet recovery = 1\n",
            {
            {Err::MissingCommaSeparatorForArgumentsInModifier, 1, 24},
            {Err::UnmatchedParenthesisAfterModifierArgs, 1, 31}
            },
            [](const Program& ast) {
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            EXPECT_EQ(dict->elements.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_on_value_instead_of_dict_key",
            "let obj = { k: @modifier 1 }\nlet recovery = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 1, 16},
            {Err::ModifiersAttachedToInvalidDeclaration, 1, 16},
            },
            [](const Program& ast) {
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            EXPECT_EQ(dict->elements.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_key_multiple_modifiers_one_missing_name_and_key",
            "let obj = { @ok @1 : 1, other: 2 }\nlet recovery = 1\n",
            {
            {Err::ExpectedModifierName, 1, 18},
            {Err::ExpectedDictionaryKey, 1, 20}
            },
            [](const Program& ast) {
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_EQ(dict->elements.size(), 2);
            EXPECT_EQ(dict->elements[0].key, "<error>");
            EXPECT_EQ(dict->elements[1].key, "other");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "func_param_modifier_missing_param_name",
            "func f(@test(a: 1) : int, b: int) -> void {}\n"
            "let recovery = 1\n",
            { {Err::MissingParameterName, 1, 20} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();

            bool found_b = false;
            for (const auto& p : func->parameters) {
            if (p.name == "b") found_b = true;
            }
            EXPECT_TRUE(found_b) << "Parser failed to recover to parameter 'b'";
            }
            },
            ParserErrorsSynchronizationTestCase{
            "func_param_modifier_missing_right_paren_in_args",
            "func f(@test(a: 1 p: int) -> void {}\n"
            "let recovery = 1\n",
            {
            {Err::MissingCommaSeparatorForArgumentsInModifier, 1, 19},
            {Err::MissingParameterName, 1, 27},
            {Err::ExpectedRightParenAfterParameters, 1, 37}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 0);

            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].name, "recovery");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "struct_field_modifier_missing_right_brace_in_struct",
            "struct S { @test(a: 1) id: int \n"
            "let recovery = 1\n",
            {
            {Err::ExpectedRightBraceAfterStructBody, 1, 31}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            EXPECT_EQ(ast.struct_definitions[0]->name, "S");

            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].name, "recovery");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "struct_field_modifier_unmatched_paren_stretches_to_eof",
            "struct S { @test(a: 1 \n",
            {
            {Err::UnmatchedParenthesisAfterModifierArgs, 1, 22},
            {Err::ExpectedStructFieldName, 1, 22},
            {Err::ExpectedRightBraceAfterStructBody, 1, 22}
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 0);
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
        return test_info.param.test_name;
        }
    );
}
