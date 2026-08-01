#include "frontend/parser/helpers/parser_test_base.h"

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

            reg({
                .name = "ModifierMissingArgColon",
                .code = "@test(a 1, b: 2)",
                .errors = {{E::MissingColonAfterArgument, 1, 9, 1, 10}},
                .verifier = std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"<error>", IsNull()},
                            {"b", IsNumber("2")}
                        }
                    }
                }
            });

            reg({
                .name = "ModifierMissingArgName",
                .code = "@test(:1, b: 2)",
                .errors = {{E::MissingArgumentNameInModifier, 1, 7, 1, 8}},
                .verifier = std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"<error>", IsNumber("1")},
                            {"b", IsNumber("2")}
                        }
                    }
                }
            });

            reg({
                .name = "ModifierMissingName",
                .code = "@*",
                .errors = {{E::ExpectedModifierName, 1, 2, 1, 3}},
                .verifier = std::vector<ModifierSpec>{{"<error>", {}}}
            });

            reg({
                .name = "ModifierGarbageInArgs",
                .code = "@test(a: 1, !, b: 2)",
                .errors = {
                    {LexerErrorCode::InvalidCharacter, 1, 13, 1, 14},
                    {E::MissingArgumentNameInModifier, 1, 14, 1, 15}
                },
                .verifier = std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"a", IsNumber("1")},
                            {"<error>", IsNull()},
                            {"b", IsNumber("2")}
                        }
                    }
                }
            });

            reg({
                .name = "MultipleModifiersOneBroken",
                .code = "@valid @broken(missing_colon) @another",
                .errors = {{E::MissingColonAfterArgument, 1, 29, 1, 30}},
                .verifier = std::vector<ModifierSpec>{
                    {"valid", {}},
                    {"broken", {{"missing_colon", IsNull()}}},
                    {"another", {}}
                }
            });

            reg({
                .name = "ModifierArgMissingColonMangle",
                .code = "@test(a: 1, b, c: 3)",
                .errors = {{E::MissingColonAfterArgument, 1, 14, 1, 15}},
                .verifier = std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"a", IsNumber("1")},
                            {"b", IsNull()},
                            {"c", IsNumber("3")}
                        }
                    }
                }
            });

            reg({
                .name = "ModifierArgValueMissing",
                .code = "@test(a: , b: 2)",
                .errors = {{E::InvalidExpression, 1, 10, 1, 11}},
                .verifier = std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"a", IsNull()},
                            {"b", IsNumber("2")}
                        }
                    }
                }
            });

            reg({
                .name = "ModifierDoubleComma",
                .code = "@test(a: 1,, b: 2)",
                .errors = {{E::MissingArgumentNameInModifier, 1, 12, 1, 13}},
                .verifier = std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"a", IsNumber("1")},
                            {"<error>", IsNull()},
                            {"b", IsNumber("2")}
                        }
                    }
                }
            });

            reg({
                .name = "ModifierTrailingCommaError",
                .code = "@test(a: 1,)",
                .errors = {{E::TrailingCommaInModifier, 1, 11, 1, 12}},
                .verifier = std::vector<ModifierSpec>{
                    {"test", {{"a", IsNumber("1")}}}
                }
            });

            reg({
                .name = "ModifierArgNameIsLiteral",
                .code = "@test(123: 1)",
                .errors = {{E::MissingArgumentNameInModifier, 1, 7, 1, 10}},
                .verifier = std::vector<ModifierSpec>{
                    {"test", {{"<error>", IsNumber("1")}}}
                }
            });

            reg({
                .name = "ModifierMissingMultipleCommas",
                .code = "@test(a: 1 b: 2 c: 3)",
                .errors = {
                    {E::MissingCommaSeparatorForArgumentsInModifier, 1, 12, 1, 13},
                    {E::MissingCommaSeparatorForArgumentsInModifier, 1, 17, 1, 18}
                },
                .verifier = std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"a", IsNumber("1")},
                            {"b", IsNumber("2")},
                            {"c", IsNumber("3")}
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
        [](const testing::TestParamInfo<ErrorRegistryEntry<ModifierVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
