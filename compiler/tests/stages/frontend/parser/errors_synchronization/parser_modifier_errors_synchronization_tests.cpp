#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

using Err = valuascript::compiler::ValuascriptErrorCode;

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

        ExpectedArgument(const char *n, std::function<void(const Expression *)> v) : name(n), verifier(std::move(v)) {
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
    ParserExhaustiveStressTests,
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
        ExpectModifierSet({ })
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
        { {Err::ModifiersAttachedToInvalidDeclaration, 1, 6} },
        [](const Program& ast) {
        ASSERT_EQ(ast.execution_steps.size(), 1);
        auto* f_call = dynamic_cast<ExpressionStatement*>(ast.execution_steps[0].get());
        ASSERT_EQ(f_call, nullptr);
        }
        },

        ParserErrorsSynchronizationTestCase{
        "modifier_on_return_in_func",
        "func f() -> int { @test return 1 }\n"
        "let recovery = 1\n",
        { {Err::ModifiersOnNonVariableDeclaration, 1, 25} },
        [](const Program& ast) {
        ASSERT_EQ(ast.function_definitions.size(), 0);
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
        ASSERT_EQ(dict_arg->pairs.size(), 1);
        ASSERT_EQ(dict_arg->pairs[0].first, "a");
        auto const num_literal = dynamic_cast<NumberLiteral*>(dict_arg->pairs[0].second.get());
        ASSERT_EQ(num_literal->value, "1");
        }
        },
        ParserErrorsSynchronizationTestCase{
        "modifier_arg_with_broken_switch",
        "@test(val: switch(x) { case A -> }) let a = 1\n"
        "let recovery = 1\n",
        { {Err::InvalidExpression, 1, 34} },
        ExpectModifierSet({ {"test", {}} })
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
        { {Err::UnmatchedParenthesisAfterModifierArgs, 1, 11} },
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
        {Err::InvalidExpression, 1, 13},
        {Err::ModifiersAttachedToInvalidDeclaration, 1, 19},
        },
        [](const Program& ast) {
        EXPECT_EQ(ast.execution_steps.size(), 1);
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
        { {Err::ModifiersAttachedToInvalidDeclaration, 1, 7} },
        [](const Program& ast) {
        ASSERT_EQ(ast.directives.size(), 1);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "modifier_on_import",
        "@test import \"module.vs\"\n"
        "let recovery = 1\n",
        { {Err::ModifiersAttachedToInvalidDeclaration, 1, 7} },
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
        { {Err::ModifiersAttachedToInvalidDeclaration, 2, 7} },
        [](const Program& ast) {
        EXPECT_EQ(ast.execution_steps.size(), 2);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "modifier_on_multi_assignment",
        "@test let a: int, @broken b: int = 1\n"
        "let recovery = 1\n",
        {
        {Err::InvalidIdentifier, 1, 19},
        {Err::ModifiersAttachedToInvalidDeclaration, 1, 27}
        },
        [](const Program& ast) {
        ASSERT_EQ(ast.execution_steps.size(), 1);
        auto* assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
        ASSERT_NE(assign, nullptr);
        EXPECT_EQ(assign->modifiers.size(), 0);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "modifier_arg_value_missing",
        "@test(a: , b: 2) let c = 3\n"
        "let recovery = 1\n",
        { {Err::InvalidExpression, 1, 10} },
        ExpectModifierSet({ {"test", {{"b", "2"}}} })
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
        "let a = 1\n@ ",
        { {Err::ExpectedModifierName, 2, 2} },
        [](const Program& ast) {
        ASSERT_EQ(ast.execution_steps.size(), 1);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "modifier_eof_after_paren",
        "@test(",
        { {Err::UnmatchedParenthesisAfterModifierArgs, 1, 7} },
        [](const Program& ast) {
        ASSERT_EQ(ast.execution_steps.size(), 0);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "modifier_arg_with_broken_binary_expr",
        "@test(a: 1 + (2 *)) let a = 1\n"
        "let recovery = 1\n",
        { {Err::InvalidExpression, 1, 18} },
        ExpectModifierSet({ {"test", {}} })
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
        { {Err::ModifiersAttachedToInvalidDeclaration, 1, 6} },
        [](const Program& ast) {
        ASSERT_EQ(ast.directives.size(), 1);
        ASSERT_EQ(ast.execution_steps.size(), 2);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "modifier_on_func_param_unsupported",
        "func f(@test a: int, b: int) -> int { return 1 }\n"
        "let recovery = 1\n",
        {
        {Err::ModifiersAttachedToInvalidDeclaration, 1, 9}
        },
        [](const Program& ast) {
        EXPECT_EQ(ast.function_definitions.size(), 1);
        EXPECT_EQ(ast.function_definitions[0]->modifiers.size(), 0);
        EXPECT_EQ(ast.function_definitions[0]->parameters.size(), 2);
        EXPECT_EQ(ast.function_definitions[0]->parameters[0].name, "a");
        EXPECT_EQ(ast.function_definitions[0]->parameters[1].name, "b");
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
        ExpectModifierSet({ {"test", {}} })
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
        {Err::InvalidExpression, 1, 12}
        },
        [](const Program& ast) {
        ASSERT_EQ(ast.execution_steps.size(), 2);
        auto step = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
        ASSERT_EQ(step->targets[0].first, "a");
        ASSERT_EQ(step->modifiers.size(), 1);
        ASSERT_EQ(step->modifiers[0].name, "func");
        ASSERT_EQ(step->modifiers[0].arguments.size(), 0);
        }
        },
        ParserErrorsSynchronizationTestCase{
        "modifier_with_comments_and_whitespace",
        "@test // line comment\n"
        "( \n a: 1 // end \n ) let a = 1\n"
        "let recovery = 1\n",
        {},
        ExpectModifierSet({ {"test", {{"a", "1"}}} })
        }
    ),
    [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
    return info.param.test_name;
    }
);
