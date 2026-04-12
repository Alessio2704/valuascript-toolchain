#include <gtest/gtest.h>
#include "frontend/parser/parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    namespace {
        const std::vector<Modifier> *GetModifiersFromFirstStmt(const Program &ast) {
            if (ast.execution_steps.empty()) return nullptr;

            auto *assign = dynamic_cast<Assignment *>(ast.execution_steps.front().get());
            if (!assign) return nullptr;

            return &assign->modifiers;
        }

        struct ExpectedArgument {
            std::string name;
            std::optional<std::string> expected_number_value;
            std::function<void(const Expression *)> verifier;

            ExpectedArgument(const char *n) : name(n), expected_number_value(std::nullopt) {
            }

            ExpectedArgument(const char *n, const char *v) : name(n), expected_number_value(std::string(v)) {
            }

            ExpectedArgument(const char *n,
                             std::function<void(const Expression *)> v) : name(n), verifier(std::move(v)) {
            }
        };

        struct ExpectedModifier {
            std::string name;
            std::vector<ExpectedArgument> args;
        };

        void ExpectModifiers(const Program &ast, const std::vector<ExpectedModifier> &expected) {
            auto *actual_mods = GetModifiersFromFirstStmt(ast);
            ASSERT_NE(actual_mods, nullptr) << "First statement is not an Assignment or has no modifiers field.";

            ASSERT_EQ(actual_mods->size(), expected.size()) << "Modifier count mismatch!";

            for (size_t i = 0; i < expected.size(); ++i) {
                EXPECT_EQ((*actual_mods)[i].name, expected[i].name) << "Modifier name mismatch at index " << i;

                const auto &actual_args = (*actual_mods)[i].arguments;
                const auto &expected_args = expected[i].args;

                ASSERT_EQ(actual_args.size(), expected_args.size())
                     << "Argument count mismatch for modifier @" << expected[i].name;

                for (size_t j = 0; j < expected_args.size(); ++j) {
                    EXPECT_EQ(actual_args[j].first, expected_args[j].name)
                         << "Arg name mismatch in @" << expected[i].name;

                    if (expected_args[j].expected_number_value.has_value()) {
                        auto *num = dynamic_cast<NumberLiteral *>(actual_args[j].second.get());
                        ASSERT_NE(num, nullptr) << "Expected number literal in @" << expected[i].name;
                        EXPECT_EQ(num->value, *expected_args[j].expected_number_value);
                    }
                }
            }
        }

        auto ExpectModifierSet(std::vector<ExpectedModifier> mods) {
            return [mods = std::move(mods)](const Program &ast) {
                ExpectModifiers(ast, mods);
                EXPECT_GT(ast.execution_steps.size(), 1);
            };
        }
    }

    class ModifierParserSynchronizationTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(ModifierParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        ModifierStressTests,
        ModifierParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            "missing_modifier_name",
            "@ let a = 1\n"
            "let recovery = 1\n",
            { {Err::ExpectedModifierName, 1, 3} },
            ExpectModifierSet({})
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_missing_closing_paren",
            "@test(a: 1 let b = 2\n"
            "let recovery = 1\n",
            { {Err::UnmatchedParenthesisAfterModifierArgs, 1, 12} },
            ExpectModifierSet({{"test", {"a"}}})
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_missing_colon",
            "@test(a 1, b: 2) let c = 3\n"
            "let recovery = 1\n",
            { {Err::MissingColonAfterArgument, 1, 9} },
            ExpectModifierSet({ {"test", {{"b", "2"}}} })
            },

            ParserErrorsSynchronizationTestCase{
            "modifier_missing_comma",
            "@test(a: 1 b: 2) let c = 3\n"
            "let recovery = 1\n",
            { {Err::MissingCommaSeparatorForArgumentsInModifier, 1, 12} },
            ExpectModifierSet({ {"test", {{"a", "1"}, {"b", "2"}}} })
            },

            ParserErrorsSynchronizationTestCase{
            "modifier_garbage_in_args",
            "@test(a: 1, !, b: 2) let c = 3\n"
            "let recovery = 1\n",
            {
            {Err::InvalidCharacter, 1, 13},
            {Err::MissingArgumentNameInModifier, 1, 14},
            },
            ExpectModifierSet({ {"test", {{"a", "1"}, {"b", "2"}}} })
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
            "modifier_on_return_in_func",
            "func f() -> int { @test return 1 }\n"
            "let recovery = 1\n",
            { {Err::ModifiersAttachedToInvalidDeclaration, 1, 19} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            ASSERT_EQ(ast.function_definitions[0]->body.size(), 1);
            auto ret_stmt = dynamic_cast<ReturnStatement*>(ast.function_definitions[0]->body[0].get());
            ASSERT_NE(ret_stmt, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_as_modifier_name",
            "@struct(a: 1) let b = 2\n"
            "let recovery = 1\n",
            { {Err::ReservedKeywordAsIdentifier, 1, 2} },
            ExpectModifierSet({ {"struct", {{"a", "1"}}} })
            },
            ParserErrorsSynchronizationTestCase{
            "multiple_modifiers_one_broken",
            "@valid @broken( missing_colon ) @another let a = 1\n"
            "let recovery = 1\n",
            { {Err::MissingColonAfterArgument, 1, 31} },
            ExpectModifierSet({
                {"valid", {}},
                {"broken", {}},
                {"another", {}}
                })
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_arg_value_is_broken_expression",
            "@test(a: 1 + *, b: 2) let c = 3\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 14} },
            ExpectModifierSet({ {"test", {{"b", "2"}}} })
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_arg_name_is_reserved_keyword",
            "@test(let: 1, func: 2) let a = 1\n"
            "let recovery = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 7},
            {Err::ReservedKeywordAsIdentifier, 1, 15}
            },
            ExpectModifierSet({ {"test", {{"let", "1"}, {"func", "2"}}} })
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_arg_with_broken_dict",
            "@test(config: { a: 1, b }) let a = 1\n"
            "let recovery = 1\n",
            { {Err::ExpectedColonAfterDictionaryKey, 1, 25} },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto const step = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_EQ(step->modifiers.size(), 1);
            ASSERT_EQ(step->modifiers[0].name, "test");
            ASSERT_EQ(step->modifiers[0].arguments.size(), 1);
            ASSERT_EQ(step->modifiers[0].arguments[0].first, "config");
            auto const dict_arg = dynamic_cast<DictLiteral*>(step->modifiers[0].arguments[0].second.get());
            ASSERT_EQ(dict_arg->elements.size(), 1);
            ASSERT_EQ(dict_arg->elements[0].key, "a");
            auto const num_literal = dynamic_cast<NumberLiteral*>(dict_arg->elements[0].value.get());
            ASSERT_EQ(num_literal->value, "1");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_arg_with_broken_switch",
            "@test(val: switch(x) { case A -> }) let a = 1\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 34} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_EQ(assign->modifiers.size(), 1);
            ASSERT_EQ(assign->modifiers[0].name, "test");
            ASSERT_EQ(assign->modifiers[0].arguments.size(), 1);
            ASSERT_EQ(assign->modifiers[0].arguments[0].first, "val");
            auto const switch_expr = dynamic_cast<SwitchExpression*>(assign->modifiers[0].arguments[0].second.get());
            EXPECT_NE(switch_expr, nullptr);
            EXPECT_EQ(switch_expr->cases.size(), 1);
            EXPECT_EQ(switch_expr->cases[0].second.get(), nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_on_broken_struct",
            "@test(a: 1) struct S { a: }\n"
            "let recovery = 1\n",
            { {Err::MissingTypeAnnotation, 1, 27} },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            EXPECT_EQ(ast.struct_definitions[0]->modifiers.size(), 1);
            EXPECT_EQ(ast.struct_definitions[0]->modifiers[0].name, "test");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_on_broken_enum",
            "@meta enum E: int { A = , B = 2 }\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 25} },
            [](const Program& ast) {
            ASSERT_EQ(ast.enum_definitions.size(), 1);
            EXPECT_EQ(ast.enum_definitions[0]->modifiers.size(), 1);
            EXPECT_EQ(ast.enum_definitions[0]->cases.size(), 1);
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
            ASSERT_EQ(ast.function_definitions[0]->parameters.size(), 0);
            EXPECT_EQ(ast.execution_steps.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "multiple_modifiers_garbage_between",
            "@first & @second let a = 1\n"
            "let recovery = 1\n",
            { {Err::InvalidCharacter, 1, 8} },
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
            "modifier_separated_by_newlines",
            "@test\n\n\n(a: 1)\nlet a = 1\n"
            "let recovery = 1\n",
            {},
            ExpectModifierSet({ {"test", {{"a", "1"}}} })
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_inside_expression_context",
            "let a = 1 + @test 2\n"
            "let recovery = 1\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 1, 13},
            {Err::ModifiersAttachedToInvalidDeclaration, 1, 13},
            },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto binary_exp = dynamic_cast<BinaryExpression*>(assign->value.get());
            ASSERT_NE(binary_exp, nullptr);
            ASSERT_EQ(binary_exp->op, TokenType::Plus);
            auto left = dynamic_cast<NumberLiteral*>(binary_exp->left.get());
            ASSERT_EQ(left->value, "1");
            auto right = dynamic_cast<NumberLiteral*>(binary_exp->right.get());
            ASSERT_EQ(right->value, "2");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_arg_missing_colon_full_mangle",
            "@test(a: 1, b, c: 3) let x = 1\n"
            "let recovery = 1\n",
            { {Err::MissingColonAfterArgument, 1, 14} },
            ExpectModifierSet({ {"test", {{"a", "1"}, {"c", "3"}}} })
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
            "modifier_on_import",
            "@test import \"module.vs\"\n"
            "let recovery = 1\n",
            { {Err::ModifiersAttachedToInvalidDeclaration, 1, 1} },
            [](const Program& ast) {
            ASSERT_EQ(ast.import_statements.size(), 1);
            EXPECT_EQ(ast.import_statements[0]->path, "\"module.vs\"");
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
            "modifier_on_multi_assignment_1",
            "@test let a: int, @broken b: int = 1\n"
            "let recovery = 1\n",
            {
            {Err::ModifiersAttachedToMultiAssignmentSingleElements, 1, 19}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto* assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            ASSERT_EQ(assign->targets.size(), 2);
            EXPECT_EQ(assign->modifiers.size(), 1);
            EXPECT_EQ(assign->modifiers[0].name, "test");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_on_multi_assignment_2",
            "@test let a: int, @one @two @three b: int = 1\n"
            "let recovery = 1\n",
            {
            {Err::ModifiersAttachedToMultiAssignmentSingleElements, 1, 19}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto* assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            ASSERT_EQ(assign->targets.size(), 2);
            EXPECT_EQ(assign->modifiers.size(), 1);
            EXPECT_EQ(assign->modifiers[0].name, "test");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_on_multi_assignment_3",
            "@test let a: int, @one @two @three b: int, @four c = 1\n"
            "let recovery = 1\n",
            {
            {Err::ModifiersAttachedToMultiAssignmentSingleElements, 1, 19},
            {Err::ModifiersAttachedToMultiAssignmentSingleElements, 1, 44},
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto* assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            ASSERT_EQ(assign->targets.size(), 3);
            EXPECT_EQ(assign->modifiers.size(), 1);
            EXPECT_EQ(assign->modifiers[0].name, "test");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_on_multi_assignment_4",
            "@test let a: int, @one @two @three(a: 1) b: int, @four c = 1\n"
            "let recovery = 1\n",
            {
            {Err::ModifiersAttachedToMultiAssignmentSingleElements, 1, 19},
            {Err::ModifiersAttachedToMultiAssignmentSingleElements, 1, 50},
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto* assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            ASSERT_EQ(assign->targets.size(), 3);
            EXPECT_EQ(assign->modifiers.size(), 1);
            EXPECT_EQ(assign->modifiers[0].name, "test");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_arg_value_missing",
            "@test(a: , b: 2) let c = 3\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 10} },
            [](const Program &ast) {
            auto assign_expr = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign_expr, nullptr);
            auto& modifier = assign_expr->modifiers[0];
            EXPECT_EQ(modifier.arguments.size(), 2);
            EXPECT_EQ(modifier.name, "test");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_double_comma",
            "@test(a: 1,, b: 2) let c = 3\n"
            "let recovery = 1\n",
            { {Err::MissingArgumentNameInModifier, 1, 12} },
            ExpectModifierSet({ {"test", {{"a", "1"}, {"b", "2"}}} })
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_trailing_comma_error",
            "@test(a: 1,) let c = 3\n"
            "let recovery = 1\n",
            { {Err::TrailingCommaInModifier, 1, 11} },
            ExpectModifierSet({ {"test", {{"a", "1"}}} })
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_arg_name_is_literal",
            "@test(123: 1) let a = 1\n"
            "let recovery = 1\n",
            { {Err::MissingArgumentNameInModifier, 1, 7} },
            ExpectModifierSet({ {"test", {}} })
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_newline_between_at_and_name",
            "@\n  test(a: 1) let a = 1\n"
            "let recovery = 1\n",
            {},
            ExpectModifierSet({ {"test", {{"a", "1"}}} })
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_eof_after_at",
            "let a = 1\n@",
            { {Err::ExpectedModifierName, 2, 2} },
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
            "modifier_arg_with_broken_binary_expr",
            "@test(a: 1 + (2 *)) let a = 1\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 18} },
            [](const Program& ast) {
            auto const step = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_EQ(step->modifiers[0].arguments.size(), 1);
            ASSERT_EQ(step->modifiers[0].arguments[0].first, "a");
            auto const binary_exp = dynamic_cast<BinaryExpression*>(step->modifiers[0].arguments[0].second.get());
            ASSERT_NE(binary_exp, nullptr);
            ASSERT_EQ(binary_exp->op, TokenType::Plus);

            auto const binary_exp_left = dynamic_cast<NumberLiteral*>(binary_exp->left.get());
            auto const binary_exp_right = dynamic_cast<GroupingExpression*>(binary_exp->right.get());
            ASSERT_NE(binary_exp_left, nullptr);
            ASSERT_NE(binary_exp_right, nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_arg_with_broken_tensor",
            "@test(vec: [1, 2, $]) let a = 1\n"
            "let recovery = 1\n",
            { {Err::InvalidCharacter, 1, 19} },
            [](const Program& ast) {
            auto const step = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_EQ(step->modifiers[0].arguments[0].first, "vec");
            auto const tensor = dynamic_cast<TensorLiteral*>(step->modifiers[0].arguments[0].second.get());
            ASSERT_EQ(tensor->elements.size(), 2);
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
            "modifier_name_is_combined_keywords",
            "@let_var let a = 1\n"
            "let recovery = 1\n",
            {},
            ExpectModifierSet({ {"let_var", {}} })
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_with_deeply_nested_sync_fail",
            "@test(a: switch(x) { case A -> switch(y) { case B -> } }) let a = 1\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 54} },
            [](const Program& ast) {
            EXPECT_EQ(ast.execution_steps.size(), 2);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_EQ(assign->modifiers.size(), 1);
            ASSERT_EQ(assign->modifiers[0].name, "test");
            ASSERT_EQ(assign->modifiers[0].arguments.size(), 1);
            ASSERT_EQ(assign->modifiers[0].arguments[0].first, "a");
            auto const switch_expr = dynamic_cast<SwitchExpression*>(assign->modifiers[0].arguments[0].second.get());
            EXPECT_NE(switch_expr, nullptr);
            EXPECT_EQ(switch_expr->cases.size(), 1);
            auto const switch_expr_2 = dynamic_cast<SwitchExpression*>(switch_expr->cases[0].second.get());
            EXPECT_NE(switch_expr_2, nullptr);
            EXPECT_EQ(switch_expr_2->cases.size(), 1);
            EXPECT_EQ(switch_expr_2->cases[0].second.get(), nullptr);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifiers_with_missing_commas_between_args",
            "@test(a: 1 b: 2 c: 3) let x = 1\n"
            "let recovery = 1\n",
            {
            {Err::MissingCommaSeparatorForArgumentsInModifier, 1, 12},
            {Err::MissingCommaSeparatorForArgumentsInModifier, 1, 17}
            },
            ExpectModifierSet({ {"test", {{"a", "1"}, {"b", "2"}, {"c", "3"}}} })
            },
            ParserErrorsSynchronizationTestCase{
            "all_identifiers_are_reserved",
            "@func(let: var) let a = 1\n"
            "let recovery = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 2},
            {Err::ReservedKeywordAsIdentifier, 1, 7},
            {Err::ReservedKeywordAsIdentifier, 1, 12}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.execution_steps.size(), 2);
            auto step = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_EQ(step->targets[0].first, "a");
            ASSERT_EQ(step->modifiers.size(), 1);
            ASSERT_EQ(step->modifiers[0].name, "func");
            ASSERT_EQ(step->modifiers[0].arguments.size(), 1);
            ASSERT_EQ(step->modifiers[0].arguments[0].first, "let");
            auto arg_value = dynamic_cast<IdentifierAccess*>(step->modifiers[0].arguments[0].second.get());
            ASSERT_EQ(arg_value->name, "var");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "modifier_with_comments_and_whitespace",
            "@test // line comment\n"
            "( \n a: 1 // end \n ) let a = 1\n"
            "let recovery = 1\n",
            {},
            ExpectModifierSet({ {"test", {{"a", "1"}}} })
            },
            ParserErrorsSynchronizationTestCase{
            "dict_key_modifier_missing_name",
            "let obj = { @* a: 1, other: 2 }\nlet recovery = 1\n",
            { {Err::ExpectedModifierName, 1, 14} },
            [](const Program& ast) {
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_EQ(dict->elements.size(), 2);
            EXPECT_EQ(dict->elements[0].key, "a");
            EXPECT_EQ(dict->elements[1].key, "other");
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
            ASSERT_EQ(dict->elements.size(), 1);
            EXPECT_EQ(dict->elements[0].key, "other");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_key_modifier_broken_args_recovers_to_next_key",
            "let obj = { @test(a: 1 + *) k: 1, other: 2 }\nlet recovery = 1\n",
            { {Err::InvalidExpression, 1, 26} },
            [](const Program& ast) {
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_EQ(dict->elements.size(), 2);
            EXPECT_EQ(dict->elements[0].key, "k");
            ASSERT_EQ(dict->elements[0].modifiers.size(), 1);
            EXPECT_EQ(dict->elements[0].modifiers[0].name, "test");
            EXPECT_EQ(dict->elements[0].modifiers[0].arguments.size(), 0);
            EXPECT_EQ(dict->elements[1].key, "other");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_key_modifier_missing_comma_in_args",
            "let obj = { @test(a: 1 b: 2) k: 1 }\nlet recovery = 1\n",
            { {Err::MissingCommaSeparatorForArgumentsInModifier, 1, 24} },
            [](const Program& ast) {
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_EQ(dict->elements.size(), 1);
            EXPECT_EQ(dict->elements[0].key, "k");
            ASSERT_EQ(dict->elements[0].modifiers.size(), 1);
            EXPECT_EQ(dict->elements[0].modifiers[0].arguments.size(), 2);
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
            EXPECT_EQ(dict->elements.size(), 0);
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
            "dict_key_multiple_modifiers_one_missing_name",
            "let obj = { @ok @* a: 1, other: 2 }\nlet recovery = 1\n",
            { {Err::ExpectedModifierName, 1, 18} },
            [](const Program& ast) {
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_EQ(dict->elements.size(), 2);
            EXPECT_EQ(dict->elements[0].key, "a");
            EXPECT_EQ(dict->elements[1].key, "other");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "dict_key_multiple_modifiers_one_missing_name_amd_key",
            "let obj = { @ok @1 : 1, other: 2 }\nlet recovery = 1\n",
            {
            {Err::ExpectedModifierName, 1, 18},
            {Err::ExpectedDictionaryKey, 1, 20}
            },
            [](const Program& ast) {
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_EQ(dict->elements.size(), 1);
            EXPECT_EQ(dict->elements[0].key, "other");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "enum_case_modifier_missing_colon_in_args",
            "enum E: int { @modifier(a 1) A = 1, B = 2 }\nlet recovery = 1\n",
            { {Err::MissingColonAfterArgument, 1, 27} },
            [](const Program& ast) {
            auto enum_def = ast.enum_definitions[0].get();
            ASSERT_EQ(enum_def->cases.size(), 2);
            EXPECT_EQ(enum_def->cases[0].name, "A");
            ASSERT_EQ(enum_def->cases[0].modifiers.size(), 1);
            EXPECT_EQ(enum_def->cases[0].modifiers[0].name, "modifier");
            EXPECT_EQ(enum_def->cases[1].name, "B");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "enum_case_modifier_name_is_reserved_keyword",
            "enum E: int { @let A = 1 }\nlet recovery = 1\n",
            { {Err::ReservedKeywordAsIdentifier, 1, 16} },
            [](const Program& ast) {
            auto enum_def = ast.enum_definitions[0].get();
            ASSERT_EQ(enum_def->cases.size(), 1);
            EXPECT_EQ(enum_def->cases[0].modifiers.size(), 1);
            EXPECT_EQ(enum_def->cases[0].modifiers[0].name, "let");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "enum_case_modifier_double_comma",
            "enum E: int { @modifier(a: 1,,) A = 1 }\nlet recovery = 1\n",
            { {Err::MissingArgumentNameInModifier, 1, 30} },
            [](const Program& ast) {
            auto enum_def = ast.enum_definitions[0].get();
            ASSERT_EQ(enum_def->cases.size(), 1);
            EXPECT_EQ(enum_def->cases[0].modifiers[0].arguments.size(), 1);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "enum_case_modifier_broken_args_recovers_to_next_case",
            "enum E: int { @modifier(a: 1 + *) A = 1, B = 2 }\nlet recovery = 1\n",
            { {Err::InvalidExpression, 1, 32} },
            [](const Program& ast) {
            auto enum_def = ast.enum_definitions[0].get();
            ASSERT_EQ(enum_def->cases.size(), 2);
            EXPECT_EQ(enum_def->cases[0].name, "A");
            ASSERT_EQ(enum_def->cases[0].modifiers.size(), 1);
            EXPECT_EQ(enum_def->cases[1].name, "B");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "reserved_keyword_modifier_name",
            "enum E: int { @let() A = 1 }\n"
            "let test = { @let() a: 1, @if() b: 2, @enum() c: 3 }\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 16},
            {Err::ReservedKeywordAsIdentifier, 2, 15},
            {Err::ReservedKeywordAsIdentifier, 2, 28},
            {Err::ReservedKeywordAsIdentifier, 2, 40}
            },[](const Program& ast) {
            ASSERT_EQ(ast.enum_definitions.size(), 1);
            auto enum_def = ast.enum_definitions[0].get();
            EXPECT_EQ(enum_def->name, "E");
            ASSERT_EQ(enum_def->cases.size(), 1);

            const auto& enum_case = enum_def->cases[0];
            EXPECT_EQ(enum_case.name, "A");
            ASSERT_EQ(enum_case.modifiers.size(), 1);
            EXPECT_EQ(enum_case.modifiers[0].name, "let");

            auto case_val = dynamic_cast<NumberLiteral*>(enum_case.value.get());
            ASSERT_NE(case_val, nullptr);
            EXPECT_EQ(case_val->value, "1");

            ASSERT_EQ(ast.execution_steps.size(), 1);
            auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->targets[0].first, "test");

            auto dict = dynamic_cast<DictLiteral*>(assign->value.get());
            ASSERT_NE(dict, nullptr);
            ASSERT_EQ(dict->elements.size(), 3);

            EXPECT_EQ(dict->elements[0].key, "a");
            ASSERT_EQ(dict->elements[0].modifiers.size(), 1);
            EXPECT_EQ(dict->elements[0].modifiers[0].name, "let");
            EXPECT_TRUE(dict->elements[0].modifiers[0].arguments.empty());
            auto val_a = dynamic_cast<NumberLiteral*>(dict->elements[0].value.get());
            ASSERT_NE(val_a, nullptr);
            EXPECT_EQ(val_a->value, "1");

            EXPECT_EQ(dict->elements[1].key, "b");
            ASSERT_EQ(dict->elements[1].modifiers.size(), 1);
            EXPECT_EQ(dict->elements[1].modifiers[0].name, "if");
            EXPECT_TRUE(dict->elements[1].modifiers[0].arguments.empty());
            auto val_b = dynamic_cast<NumberLiteral*>(dict->elements[1].value.get());
            ASSERT_NE(val_b, nullptr);
            EXPECT_EQ(val_b->value, "2");

            EXPECT_EQ(dict->elements[2].key, "c");
            ASSERT_EQ(dict->elements[2].modifiers.size(), 1);
            EXPECT_EQ(dict->elements[2].modifiers[0].name, "enum");
            EXPECT_TRUE(dict->elements[2].modifiers[0].arguments.empty());
            auto val_c = dynamic_cast<NumberLiteral*>(dict->elements[2].value.get());
            ASSERT_NE(val_c, nullptr);
            EXPECT_EQ(val_c->value, "3");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "func_param_modifier_missing_name",
            "func f(@ 1 a: int, b: int) -> void {}\n"
            "let recovery = 1\n",
            { {Err::ExpectedModifierName, 1, 10} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();

            ASSERT_EQ(func->parameters.size(), 2);
            EXPECT_EQ(func->parameters[0].name, "a");
            EXPECT_TRUE(func->parameters[0].modifiers.empty());

            EXPECT_EQ(func->parameters[1].name, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "func_param_modifier_missing_colon_in_arg",
            "func f(@test(x 1) a: int, b: int) -> void {}\n"
            "let recovery = 1\n",
            { {Err::MissingColonAfterArgument, 1, 16} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();

            ASSERT_EQ(func->parameters.size(), 2);
            EXPECT_EQ(func->parameters[0].name, "a");

            EXPECT_FALSE(func->parameters[0].modifiers.empty());

            EXPECT_EQ(func->parameters[0].modifiers[0].name, "test");
            EXPECT_TRUE(func->parameters[0].modifiers[0].arguments.empty());

            EXPECT_EQ(func->parameters[1].name, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "func_param_modifier_broken_expression_in_arg",
            "func f(@test(x: 1 + *) a: int, b: int) -> void {}\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 21} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->parameters.size(), 2);

            EXPECT_EQ(func->parameters[0].name, "a");
            ASSERT_EQ(func->parameters[0].modifiers.size(), 1);
            EXPECT_EQ(func->parameters[0].modifiers[0].name, "test");

            EXPECT_EQ(func->parameters[1].name, "b");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "func_param_reserved_keyword_as_modifier",
            "func f(@func() a: int, @let b: int) -> void {}\n"
            "let recovery = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 9},
            {Err::ReservedKeywordAsIdentifier, 1, 25}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->parameters.size(), 2);

            EXPECT_EQ(func->parameters[0].name, "a");
            ASSERT_EQ(func->parameters[0].modifiers.size(), 1);
            EXPECT_EQ(func->parameters[0].modifiers[0].name, "func");

            EXPECT_EQ(func->parameters[1].name, "b");
            ASSERT_EQ(func->parameters[1].modifiers.size(), 1);
            EXPECT_EQ(func->parameters[1].modifiers[0].name, "let");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "func_param_stacked_modifiers_one_broken",
            "func f(@valid @broken(x: *) @ok p: int) -> void {}\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 26} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->parameters.size(), 1);

            EXPECT_EQ(func->parameters[0].name, "p");
            ASSERT_EQ(func->parameters[0].modifiers.size(), 3);

            EXPECT_EQ(func->parameters[0].modifiers[0].name, "valid");
            EXPECT_EQ(func->parameters[0].modifiers[1].name, "broken");
            EXPECT_EQ(func->parameters[0].modifiers[2].name, "ok");
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
            "func_param_modifier_arg_missing_value_hits_paren",
            "func f(@test(a: ) b: int) -> void {}\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 17} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->parameters.size(), 1);

            EXPECT_EQ(func->parameters[0].name, "b");
            ASSERT_EQ(func->parameters[0].modifiers.size(), 1);
            EXPECT_EQ(func->parameters[0].modifiers[0].name, "test");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "func_param_modifier_missing_commas_in_args",
            "func f(@test(a: 1 b: 2 c: 3) p: int) -> void {}\n"
            "let recovery = 1\n",
            {
            {Err::MissingCommaSeparatorForArgumentsInModifier, 1, 19},
            {Err::MissingCommaSeparatorForArgumentsInModifier, 1, 24}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->parameters.size(), 1);

            EXPECT_EQ(func->parameters[0].name, "p");
            ASSERT_EQ(func->parameters[0].modifiers.size(), 1);
            EXPECT_EQ(func->parameters[0].modifiers[0].name, "test");
            EXPECT_EQ(func->parameters[0].modifiers[0].arguments.size(), 3);
            }
            },
            ParserErrorsSynchronizationTestCase{
            "func_param_modifier_with_default_value_recovery",
            "func f(@test(x: *) p: int = 10, @ok q: int = 20) -> void {}\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 17} },
            [](const Program& ast) {
            ASSERT_EQ(ast.function_definitions.size(), 1);
            auto func = ast.function_definitions[0].get();
            ASSERT_EQ(func->parameters.size(), 2);

            EXPECT_EQ(func->parameters[0].name, "p");
            ASSERT_EQ(func->parameters[0].modifiers.size(), 1);
            EXPECT_EQ(func->parameters[0].modifiers[0].name, "test");
            auto def_p = dynamic_cast<NumberLiteral*>(func->parameters[0].default_value.get());
            ASSERT_NE(def_p, nullptr);
            EXPECT_EQ(def_p->value, "10");

            EXPECT_EQ(func->parameters[1].name, "q");
            ASSERT_EQ(func->parameters[1].modifiers.size(), 1);
            EXPECT_EQ(func->parameters[1].modifiers[0].name, "ok");
            auto def_q = dynamic_cast<NumberLiteral*>(func->parameters[1].default_value.get());
            ASSERT_NE(def_q, nullptr);
            EXPECT_EQ(def_q->value, "20");
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
            EXPECT_EQ(assign->targets[0].first, "recovery");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "struct_field_modifier_missing_name",
            "struct S { @1 id: int, next: float }\n"
            "let recovery = 1\n",
            { {Err::ExpectedModifierName, 1, 13} },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            auto s = ast.struct_definitions[0].get();

            ASSERT_EQ(s->fields.size(), 2);
            EXPECT_EQ(s->fields[0].name, "id");
            EXPECT_EQ(s->fields[1].name, "next");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "struct_field_modifier_missing_colon_in_arg",
            "struct S { @meta(key \"val\") id: int, next: float }\n"
            "let recovery = 1\n",
            { {Err::MissingColonAfterArgument, 1, 22} },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            auto s = ast.struct_definitions[0].get();

            ASSERT_EQ(s->fields.size(), 2);
            EXPECT_EQ(s->fields[0].name, "id");
            EXPECT_FALSE(s->fields[0].modifiers.empty());
                EXPECT_EQ(s->fields[0].modifiers[0].name, "meta");
                EXPECT_TRUE(s->fields[0].modifiers[0].arguments.empty());

            EXPECT_EQ(s->fields[1].name, "next");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "struct_field_modifier_broken_expression_in_arg",
            "struct S { @test(val: 1 + *) id: int, next: float }\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 27} },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            auto s = ast.struct_definitions[0].get();
            ASSERT_EQ(s->fields.size(), 2);

            EXPECT_EQ(s->fields[0].name, "id");
            ASSERT_EQ(s->fields[0].modifiers.size(), 1);
            EXPECT_EQ(s->fields[0].modifiers[0].name, "test");

            EXPECT_EQ(s->fields[1].name, "next");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "struct_field_reserved_keyword_as_modifier",
            "struct S { @func id: int, @let next: float }\n"
            "let recovery = 1\n",
            {
            {Err::ReservedKeywordAsIdentifier, 1, 13},
            {Err::ReservedKeywordAsIdentifier, 1, 28}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            auto s = ast.struct_definitions[0].get();
            ASSERT_EQ(s->fields.size(), 2);

            EXPECT_EQ(s->fields[0].name, "id");
            ASSERT_EQ(s->fields[0].modifiers.size(), 1);
            EXPECT_EQ(s->fields[0].modifiers[0].name, "func");

            EXPECT_EQ(s->fields[1].name, "next");
            ASSERT_EQ(s->fields[1].modifiers.size(), 1);
            EXPECT_EQ(s->fields[1].modifiers[0].name, "let");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "struct_field_stacked_modifiers_one_broken",
            "struct S { @valid @broken(x: *) @last field: int }\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 30} },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            auto s = ast.struct_definitions[0].get();
            ASSERT_EQ(s->fields.size(), 1);

            const auto& f = s->fields[0];
            EXPECT_EQ(f.name, "field");
            ASSERT_EQ(f.modifiers.size(), 3);

            EXPECT_EQ(f.modifiers[0].name, "valid");
            EXPECT_EQ(f.modifiers[1].name, "broken");
            EXPECT_EQ(f.modifiers[2].name, "last");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "struct_field_modifier_missing_field_name",
            "struct S { @test(a: 1) : int, next: float }\n"
            "let recovery = 1\n",
            { {Err::ExpectedStructFieldName, 1, 24} },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            auto s = ast.struct_definitions[0].get();

            bool found_next = false;
            for (const auto& field : s->fields) {
            if (field.name == "next") found_next = true;
            }
            EXPECT_TRUE(found_next) << "Failed to recover to field 'next'";
            }
            },
            ParserErrorsSynchronizationTestCase{
            "struct_field_modifier_arg_missing_value_hits_brace",
            "struct S { @test(a: ) id: int }\n"
            "let recovery = 1\n",
            { {Err::InvalidExpression, 1, 21} },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            auto s = ast.struct_definitions[0].get();
            ASSERT_EQ(s->fields.size(), 1);
            EXPECT_EQ(s->fields[0].name, "id");
            ASSERT_EQ(s->fields[0].modifiers.size(), 1);
            EXPECT_EQ(s->fields[0].modifiers[0].name, "test");
            }
            },
            ParserErrorsSynchronizationTestCase{
            "struct_field_modifier_missing_commas_in_args",
            "struct S { @test(a: 1 b: 2 c: 3) id: int }\n"
            "let recovery = 1\n",
            {
            {Err::MissingCommaSeparatorForArgumentsInModifier, 1, 23},
            {Err::MissingCommaSeparatorForArgumentsInModifier, 1, 28}
            },
            [](const Program& ast) {
            ASSERT_EQ(ast.struct_definitions.size(), 1);
            auto s = ast.struct_definitions[0].get();
            ASSERT_EQ(s->fields.size(), 1);

            EXPECT_EQ(s->fields[0].name, "id");
            ASSERT_EQ(s->fields[0].modifiers.size(), 1);
            EXPECT_EQ(s->fields[0].modifiers[0].name, "test");
            EXPECT_EQ(s->fields[0].modifiers[0].arguments.size(), 3);
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
            EXPECT_EQ(assign->targets[0].first, "recovery");
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
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
