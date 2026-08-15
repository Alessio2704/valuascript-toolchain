#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_names.h"

namespace valuascript::compiler::test
{
    class ModifierErrorRegistryRunner : public ParserTestBase,
                                        public testing::WithParamInterface<ErrorRegistryEntry<ModifierVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<ModifierVerifier>& spec) { ErrorRegistry::add(spec); };

            reg(RecoveryCase<ModifierVerifier>{
                .name = "ModifierMissingArgColon",
                .code = "@test(a 1, b: 2)",
                .errors = {PErr{.code = E::MissingColonAfterArgument, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10}},
                .verifier = std::vector<ModifierSpec>{
                    {
                        .name="test", .args={
                            {.label="<error>", .value_v=IsNull()},
                            {.label="b", .value_v=IsNumber("2")}
                        }
                    }
                }
            });

            reg(RecoveryCase<ModifierVerifier>{
                .name = "ModifierMissingArgName",
                .code = "@test(:1, b: 2)",
                .errors = {PErr{.code = E::MissingArgumentNameInModifier, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}},
                .verifier = std::vector<ModifierSpec>{
                    {
                        .name="test", .args={
                            {.label="<error>", .value_v=IsNumber("1")},
                            {.label="b", .value_v=IsNumber("2")}
                        }
                    }
                }
            });

            reg(RecoveryCase<ModifierVerifier>{
                .name = "ModifierMissingName",
                .code = "@*",
                .errors = {PErr{.code = E::ExpectedModifierName, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3}},
                .verifier = std::vector<ModifierSpec>{{.name="<error>", .args={}}}
            });

            reg(RecoveryCase<ModifierVerifier>{
                .name = "ModifierGarbageInArgs",
                .code = "@test(a: 1, !, b: 2)",
                .errors = {
                    PErr{.code = LexerErrorCode::InvalidCharacter, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14},
                    PErr{.code = E::MissingArgumentNameInModifier, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}
                },
                .verifier = std::vector<ModifierSpec>{
                    {
                        .name="test", .args={
                            {.label="a", .value_v=IsNumber("1")},
                            {.label="<error>", .value_v=IsNull()},
                            {.label="b", .value_v=IsNumber("2")}
                        }
                    }
                }
            });

            reg(RecoveryCase<ModifierVerifier>{
                .name = "MultipleModifiersOneBroken",
                .code = "@valid @broken(missing_colon) @another",
                .errors = {PErr{.code = E::MissingColonAfterArgument, .line_start = 1, .column_start = 29, .line_end = 1, .column_end = 30}},
                .verifier = std::vector<ModifierSpec>{
                    {.name="valid", .args={}},
                    {.name="broken", .args={{.label="missing_colon", .value_v=IsNull()}}},
                    {.name="another", .args={}}
                }
            });

            reg(RecoveryCase<ModifierVerifier>{
                .name = "ModifierArgMissingColonMangle",
                .code = "@test(a: 1, b, c: 3)",
                .errors = {PErr{.code = E::MissingColonAfterArgument, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}},
                .verifier = std::vector<ModifierSpec>{
                    {
                        .name="test", .args={
                            {.label="a", .value_v=IsNumber("1")},
                            {.label="b", .value_v=IsNull()},
                            {.label="c", .value_v=IsNumber("3")}
                        }
                    }
                }
            });

            reg(RecoveryCase<ModifierVerifier>{
                .name = "ModifierArgValueMissing",
                .code = "@test(a: , b: 2)",
                .errors = {PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11}},
                .verifier = std::vector<ModifierSpec>{
                    {
                        .name="test", .args={
                            {.label="a", .value_v=IsNull()},
                            {.label="b", .value_v=IsNumber("2")}
                        }
                    }
                }
            });

            reg(RecoveryCase<ModifierVerifier>{
                .name = "ModifierDoubleComma",
                .code = "@test(a: 1,, b: 2)",
                .errors = {PErr{.code = E::MissingArgumentNameInModifier, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}},
                .verifier = std::vector<ModifierSpec>{
                    {
                        .name="test", .args={
                            {.label="a", .value_v=IsNumber("1")},
                            {.label="<error>", .value_v=IsNull()},
                            {.label="b", .value_v=IsNumber("2")}
                        }
                    }
                }
            });

            reg(RecoveryCase<ModifierVerifier>{
                .name = "ModifierTrailingCommaError",
                .code = "@test(a: 1,)",
                .errors = {PErr{.code = E::TrailingCommaInModifier, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}},
                .verifier = std::vector<ModifierSpec>{
                    {.name="test", .args={{.label="a", .value_v=IsNumber("1")}}}
                }
            });

            reg(RecoveryCase<ModifierVerifier>{
                .name = "ModifierArgNameIsLiteral",
                .code = "@test(123: 1)",
                .errors = {PErr{.code = E::MissingArgumentNameInModifier, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 10}},
                .verifier = std::vector<ModifierSpec>{
                    {.name="test", .args={{.label="<error>", .value_v=IsNumber("1")}}}
                }
            });

            reg(RecoveryCase<ModifierVerifier>{
                .name = "ModifierMissingMultipleCommas",
                .code = "@test(a: 1 b: 2 c: 3)",
                .errors = {
                    PErr{.code = E::MissingCommaSeparatorForArgumentsInModifier, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13},
                    PErr{.code = E::MissingCommaSeparatorForArgumentsInModifier, .line_start = 1, .column_start = 17, .line_end = 1, .column_end = 18}
                },
                .verifier = std::vector<ModifierSpec>{
                    {
                        .name="test", .args={
                            {.label="a", .value_v=IsNumber("1")},
                            {.label="b", .value_v=IsNumber("2")},
                            {.label="c", .value_v=IsNumber("3")}
                        }
                    }
                }
            });

            reg(RecoveryCase<ModifierVerifier>{
                .name = "ModifierMissingClosingParen",
                .code = "@test(a: 1",
                .errors = {
                    PErr{.code = E::UnmatchedParenthesisAfterModifierArgs, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11}
                },
                .verifier = std::vector<ModifierSpec>{
                    {
                        .name = "test",
                        .args = {
                            {.label = "a", .value_v = IsNumber("1")}
                        }
                    }
                }
            });

            return true;
        }();
    }

    TEST_P(ModifierErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectModifierErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        Modifier,
        ModifierErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::modifiers()),
        TestNameGenerator{}
    );
}
