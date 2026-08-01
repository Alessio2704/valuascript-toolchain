#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class EnumErrorRegistryRunner : public ParserTestBase,
                                    public testing::WithParamInterface<ErrorRegistryEntry<EnumVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<EnumVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "EnumMissingName",
                .code = "enum : int { A }",
                .errors = {
                    {E::ExpectedEnumName, 1, 6, 1, 7}
                },
                .verifier = IsEnumDef("<error>", {}, IsType("int"), {
                    {"A"}
                })
            });

            reg({
                .name = "EnumMissingTypeAnnotation",
                .code = "enum Test: { A }",
                .errors = {
                    {E::MissingTypeAnnotation, 1, 12, 1, 13}
                },
                .verifier = IsEnumDef("Test", {}, IsNullType(), {
                    {"A"}
                })
            });

            reg({
                .name = "NoCommasASTAllCases",
                .code = "enum Test: int { A B C }",
                .errors = {
                    {E::ExpectedCommaSeparatorInEnum, 1, 20, 1, 21},
                    {E::ExpectedCommaSeparatorInEnum, 1, 22, 1, 23}
                },
                .verifier = IsEnumDef("Test", {}, IsType("int"), {
                    {"A"},
                    {"B"},
                    {"C"}
                })
            });

            reg({
                .name = "NoCommasASTAllCasesWithDefault",
                .code = "enum Test: int { A = 1 B = a() C = 2 }",
                .errors = {
                    {E::ExpectedCommaSeparatorInEnum, 1, 24, 1, 25},
                    {E::ExpectedCommaSeparatorInEnum, 1, 32, 1, 33}
                },
                .verifier = IsEnumDef("Test", {}, IsType("int"), {
                    {"A", {}, IsNumber("1")},
                    {"B", {}, IsCall(IsIdentifier("a"))},
                    {"C", {}, IsNumber("2")}
                })
            });

            reg({
                .name = "NumberCaseName",
                .code = "enum Test: int { 1 }",
                .errors = {
                    {E::ExpectedEnumCaseName, 1, 18, 1, 19}
                },
                .verifier = IsEnumDef("Test", {}, IsType("int"), {
                    {"<error>"}
                })
            });

            reg({
                .name = "StringCaseName",
                .code = "enum Test: int { \"A\" }",
                .errors = {
                    {E::ExpectedEnumCaseName, 1, 18, 1, 21}
                },
                .verifier = IsEnumDef("Test", {}, IsType("int"), {
                    {"<error>"}
                })
            });

            reg({
                .name = "CasesGarbageStart",
                .code = "enum Test: int { +-*/, B, C }",
                .errors = {
                    {E::ExpectedEnumCaseName, 1, 18, 1, 19}
                },
                .verifier = IsEnumDef("Test", {}, IsType("int"), {
                    {"<error>"},
                    {"B"},
                    {"C"}
                })
            });

            reg({
                .name = "CasesGarbageMiddle",
                .code = "enum Test: int { A, +-*/, C }",
                .errors = {
                    {E::ExpectedEnumCaseName, 1, 21, 1, 22}
                },
                .verifier = IsEnumDef("Test", {}, IsType("int"), {
                    {"A"},
                    {"<error>"},
                    {"C"}
                })
            });

            reg({
                .name = "CasesGarbageEnd",
                .code = "enum Test: int { A, B, +-*/ }",
                .errors = {
                    {E::ExpectedEnumCaseName, 1, 24, 1, 25}
                },
                .verifier = IsEnumDef("Test", {}, IsType("int"), {
                    {"A"},
                    {"B"},
                    {"<error>"}
                })
            });

            reg({
                .name = "CasesSkip",
                .code = "enum Test: int { A, , C }",
                .errors = {
                    {E::ExpectedEnumCaseName, 1, 21, 1, 22}
                },
                .verifier = IsEnumDef("Test", {}, IsType("int"), {
                    {"A"},
                    {"<error>"},
                    {"C"}
                })
            });

            reg({
                .name = "MissingDefault",
                .code = "enum Test: int { A =, B }",
                .errors = {
                    {E::InvalidExpression, 1, 21, 1, 22}
                },
                .verifier = IsEnumDef("Test", {}, IsType("int"), {
                    {"A"},
                    {"B"}
                })
            });

            reg({
                .name = "MissingDefaultLast",
                .code = "enum Test: int { A, B = }",
                .errors = {
                    {E::InvalidExpression, 1, 25, 1, 26}
                },
                .verifier = IsEnumDef("Test", {}, IsType("int"), {
                    {"A"},
                    {"B"}
                })
            });

            reg({
                .name = "MissingDefaultMultiple",
                .code = "enum Test: int { A =, B = 1, C =, D = 1, E =, F = 1}",
                .errors = {
                    {E::InvalidExpression, 1, 21, 1, 22},
                    {E::InvalidExpression, 1, 33, 1, 34},
                    {E::InvalidExpression, 1, 45, 1, 46}
                },
                .verifier = IsEnumDef("Test", {}, IsType("int"), {
                    {"A"},
                    {"B", {}, IsNumber("1")},
                    {"C"},
                    {"D", {}, IsNumber("1")},
                    {"E"},
                    {"F", {}, IsNumber("1")}
                })
            });

            return true;
        }();
    }

    TEST_P(EnumErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectEnumDefinitionErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        Enum,
        EnumErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::enums()),
        [](const testing::TestParamInfo<ErrorRegistryEntry<EnumVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
