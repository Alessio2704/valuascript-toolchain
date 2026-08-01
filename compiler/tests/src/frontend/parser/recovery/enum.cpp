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
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<EnumVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            // reg("OnlyEnumKeyword", "enum",
            //     {{E::ExpectedEnumName, 1, 5, 1, 6}},
            //     IsEnumDef("<error>"));
            //
            // reg("MissingColon", "enum Test",
            //     {{E::ExpectedColonAfterEnumName, 1, 10, 1, 11}},
            //     IsEnumDef("Test"));
            //
            // reg("MissingUnderlyingType", "enum Test :",
            //     {{E::ExpectedColonAfterEnumName, 1, 12, 1, 13}},
            //     IsEnumDef("Test"));

            // reg("MissingLeftBrace", "enum Test: int ",
            //     {{E::ExpectedLeftBraceBeforeEnumBody, 1, 15, 1, 16}},
            //     IsEnumDef("Test", {}, IsType("int")));

            // On Windows because of a sentinel it fails (the sentinel is dot access on multiline reassignment) -> maybe generate all sentinels paths not just random...
            // reg("MissingRightBrace", "enum Test: int {",
            //     {{E::ExpectedRightBraceAfterEnumBody, 1, 17, 1, 18}},
            //     IsEnumDef("Test", {}, IsType("int")));

            reg("EnumMissingName", "enum : int { A }",
                {
                    {E::ExpectedEnumName, 1, 6, 1, 7}
                },
                IsEnumDef("<error>", {}, IsType("int"), {
                              {"A"}
                          }
                )
            );

            // reg("EnumMissingColon", "enum Test int { A }",
            //     {
            //         {E::ExpectedColonAfterEnumName, 1, 11, 1, 14}
            //     },
            //     IsEnumDef("Test", {}, IsType("int"), {
            //                   {"A"}
            //               }
            //     )
            // );

            reg("EnumMissingTypeAnnotation", "enum Test: { A }",
                {
                    {E::MissingTypeAnnotation, 1, 12, 1, 13}
                },
                IsEnumDef("Test", {}, IsNullType(), {
                              {"A"}
                          }
                )
            );

            reg("NoCommasASTAllCases", "enum Test: int { A B C }",
                {
                    {E::ExpectedCommaSeparatorInEnum, 1, 20, 1, 21},
                    {E::ExpectedCommaSeparatorInEnum, 1, 22, 1, 23}
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"A"},
                              {"B"},
                              {"C"},
                          }
                )
            );

            reg("NoCommasASTAllCasesWithDefault", "enum Test: int { A = 1 B = a() C = 2 }",
                {
                    {E::ExpectedCommaSeparatorInEnum, 1, 24, 1, 25},
                    {E::ExpectedCommaSeparatorInEnum, 1, 32, 1, 33}
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"A", {}, IsNumber("1")},
                              {"B", {}, IsCall(IsIdentifier("a"))},
                              {"C", {}, IsNumber("2")},
                          }
                )
            );

            reg("NumberCaseName", "enum Test: int { 1 }",
                {
                    {E::ExpectedEnumCaseName, 1, 18, 1, 19}
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"<error>"},
                          }
                )
            );

            reg("StringCaseName", "enum Test: int { \"A\" }",
                {
                    {E::ExpectedEnumCaseName, 1, 18, 1, 21}
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"<error>"},
                          }
                )
            );

            reg("CasesGarbageStart", "enum Test: int { +-*/, B, C }",
                {
                    {E::ExpectedEnumCaseName, 1, 18, 1, 19},
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"<error>"},
                              {"B"},
                              {"C"},
                          }
                )
            );

            reg("CasesGarbageMiddle", "enum Test: int { A, +-*/, C }",
                {
                    {E::ExpectedEnumCaseName, 1, 21, 1, 22},
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"A"},
                              {"<error>"},
                              {"C"}
                          }
                )
            );

            reg("CasesGarbageEnd", "enum Test: int { A, B, +-*/ }",
                {
                    {E::ExpectedEnumCaseName, 1, 24, 1, 25},
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"A"},
                              {"B"},
                              {"<error>"}
                          }
                )
            );

            reg("CasesSkip", "enum Test: int { A, , C }",
                {
                    {E::ExpectedEnumCaseName, 1, 21, 1, 22},
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"A"},
                              {"<error>"},
                              {"C"}
                          }
                )
            );

            reg("MissingDefault", "enum Test: int { A =, B }",
                {
                    {E::InvalidExpression, 1, 21, 1, 22},
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"A"},
                              {"B"}
                          }
                )
            );

            reg("MissingDefaultLast", "enum Test: int { A, B = }",
                {
                    {E::InvalidExpression, 1, 25, 1, 26},
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"A"},
                              {"B"}
                          }
                )
            );

            reg("MissingDefaultMultiple", "enum Test: int { A =, B = 1, C =, D = 1, E =, F = 1}",
                {
                    {E::InvalidExpression, 1, 21, 1, 22},
                    {E::InvalidExpression, 1, 33, 1, 34},
                    {E::InvalidExpression, 1, 45, 1, 46},
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"A"},
                              {"B", {}, IsNumber("1")},
                              {"C"},
                              {"D", {}, IsNumber("1")},
                              {"E"},
                              {"F", {}, IsNumber("1")},
                          }
                )
            );

            return true;
        }();
    }

    TEST_P(EnumErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectEnumDefinitionErrors(code, errors, verifier, skip_contexts, context_overrides);
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
