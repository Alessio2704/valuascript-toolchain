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
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs,
                          const OneOf<ModifierVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("ModifierMissingArgColon", "@test(a 1, b: 2)",
                {{E::MissingColonAfterArgument, 1, 9, 1, 10}},
                std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"<error>", IsNull()},
                            {"b", IsNumber("2")},
                        }
                    }
                }
            );

            reg("ModifierMissingArgName", "@test(:1, b: 2)",
                {{E::MissingArgumentNameInModifier, 1, 7, 1, 8}},
                std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"<error>", IsNumber("1")},
                            {"b", IsNumber("2")},
                        }
                    }
                }
            );

            reg("ModifierMissingName", "@*",
                {{E::ExpectedModifierName, 1, 2, 1, 3}},
                std::vector<ModifierSpec>{{"<error>", {}}}
            );

            reg("ModifierGarbageInArgs", "@test(a: 1, !, b: 2)",
                {
                    {LexerErrorCode::InvalidCharacter, 1, 13, 1, 14},
                    {E::MissingArgumentNameInModifier, 1, 14, 1, 15}
                },
                std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"a", IsNumber("1")},
                            {"<error>", IsNull()},
                            {"b", IsNumber("2")}
                        }
                    }
                }
            );

            reg("MultipleModifiersOneBroken", "@valid @broken(missing_colon) @another",
                {{E::MissingColonAfterArgument, 1, 29, 1, 30}},
                std::vector<ModifierSpec>{
                    {"valid", {}},
                    {"broken", {{"missing_colon", IsNull()}}},
                    {"another", {}}
                }
            );

            reg("ModifierArgMissingColonMangle", "@test(a: 1, b, c: 3)",
                {{E::MissingColonAfterArgument, 1, 14, 1, 15}},
                std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"a", IsNumber("1")},
                            {"b", IsNull()},
                            {"c", IsNumber("3")}
                        }
                    }
                }
            );

            reg("ModifierArgValueMissing", "@test(a: , b: 2)",
                {{E::InvalidExpression, 1, 10, 1, 11}},
                std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"a", IsNull()},
                            {"b", IsNumber("2")}
                        }
                    }
                }
            );

            reg("ModifierDoubleComma", "@test(a: 1,, b: 2)",
                {{E::MissingArgumentNameInModifier, 1, 12, 1, 13}},
                std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"a", IsNumber("1")},
                            {"<error>", IsNull()},
                            {"b", IsNumber("2")}
                        }
                    }
                }
            );

            reg("ModifierTrailingCommaError", "@test(a: 1,)",
                {{E::TrailingCommaInModifier, 1, 11, 1, 12}},
                std::vector<ModifierSpec>{
                    {"test", {{"a", IsNumber("1")}}}
                }
            );

            reg("ModifierArgNameIsLiteral", "@test(123: 1)",
                {{E::MissingArgumentNameInModifier, 1, 7, 1, 10}},
                std::vector<ModifierSpec>{
                    {"test", {{"<error>", IsNumber("1")}}}
                }
            );

            reg("ModifierMissingMultipleCommas", "@test(a: 1 b: 2 c: 3)",
                {
                    {E::MissingCommaSeparatorForArgumentsInModifier, 1, 12, 1, 13},
                    {E::MissingCommaSeparatorForArgumentsInModifier, 1, 17, 1, 18}
                },
                std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"a", IsNumber("1")},
                            {"b", IsNumber("2")},
                            {"c", IsNumber("3")}
                        }
                    }
                }
            );

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
