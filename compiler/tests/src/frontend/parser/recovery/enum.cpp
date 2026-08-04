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
                    PErr{.code = E::ExpectedEnumName, .line_start = 1, .column_start = 6, .line_end = 1, .column_end = 7}
                },
                .verifier = IsEnumDef("<error>", {}, IsType("int"), {
                    {"A"}
                })
            });

            reg({
                .name = "EnumMissingTypeAnnotation",
                .code = "enum Test: { A }",
                .errors = {
                    PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                },
                .verifier = IsEnumDef("Test", {}, IsNullType(), {
                    {"A"}
                })
            });

            reg({
                .name = "NoCommasASTAllCases",
                .code = "enum Test: int { A B C }",
                .errors = {
                    PErr{.code = E::ExpectedCommaSeparatorInEnum, .line_start = 1, .column_start = 20, .line_end = 1, .column_end = 21},
                    PErr{.code = E::ExpectedCommaSeparatorInEnum, .line_start = 1, .column_start = 22, .line_end = 1, .column_end = 23}
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
                    PErr{.code = E::ExpectedCommaSeparatorInEnum, .line_start = 1, .column_start = 24, .line_end = 1, .column_end = 25},
                    PErr{.code = E::ExpectedCommaSeparatorInEnum, .line_start = 1, .column_start = 32, .line_end = 1, .column_end = 33}
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
                    PErr{.code = E::ExpectedEnumCaseName, .line_start = 1, .column_start = 18, .line_end = 1, .column_end = 19}
                },
                .verifier = IsEnumDef("Test", {}, IsType("int"), {
                    {"<error>"}
                })
            });

            reg({
                .name = "StringCaseName",
                .code = "enum Test: int { \"A\" }",
                .errors = {
                    PErr{.code = E::ExpectedEnumCaseName, .line_start = 1, .column_start = 18, .line_end = 1, .column_end = 21}
                },
                .verifier = IsEnumDef("Test", {}, IsType("int"), {
                    {"<error>"}
                })
            });

            reg({
                .name = "CasesGarbageStart",
                .code = "enum Test: int { +-*/, B, C }",
                .errors = {
                    PErr{.code = E::ExpectedEnumCaseName, .line_start = 1, .column_start = 18, .line_end = 1, .column_end = 19}
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
                    PErr{.code = E::ExpectedEnumCaseName, .line_start = 1, .column_start = 21, .line_end = 1, .column_end = 22}
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
                    PErr{.code = E::ExpectedEnumCaseName, .line_start = 1, .column_start = 24, .line_end = 1, .column_end = 25}
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
                    PErr{.code = E::ExpectedEnumCaseName, .line_start = 1, .column_start = 21, .line_end = 1, .column_end = 22}
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
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 21, .line_end = 1, .column_end = 22}
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
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 25, .line_end = 1, .column_end = 26}
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
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 21, .line_end = 1, .column_end = 22},
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 33, .line_end = 1, .column_end = 34},
                    PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 45, .line_end = 1, .column_end = 46}
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
        TestNameGenerator{}
    );
}
