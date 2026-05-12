#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class EnumErrorRegistryRunner : public ParserTestBase,
                                    public testing::WithParamInterface<ErrorRegistryEntry<EnumVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<EnumVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            // reg("OnlyEnumKeyword", "enum",
            //     {{ValuascriptErrorCode::ExpectedEnumName, 1, 5, 1, 6}},
            //     IsEnumDef("<error>"));
            //
            // reg("MissingColon", "enum Test",
            //     {{ValuascriptErrorCode::ExpectedColonAfterEnumName, 1, 10, 1, 11}},
            //     IsEnumDef("Test"));
            //
            // reg("MissingUnderlyingType", "enum Test :",
            //     {{ValuascriptErrorCode::ExpectedColonAfterEnumName, 1, 12, 1, 13}},
            //     IsEnumDef("Test"));

            // reg("MissingLeftBrace", "enum Test: int ",
            //     {{ValuascriptErrorCode::ExpectedLeftBraceBeforeEnumBody, 1, 15, 1, 16}},
            //     IsEnumDef("Test", {}, IsType("int")));

            reg("MissingRightBrace", "enum Test: int {",
                {{ValuascriptErrorCode::ExpectedRightBraceAfterEnumBody, 1, 17, 1, 18}},
                IsEnumDef("Test", {}, IsType("int")));

            reg("NoCommasASTAllCases", "enum Test: int { A B C }",
                {
                    {ValuascriptErrorCode::ExpectedCommaSeparatorInEnum, 1, 20, 1, 21},
                    {ValuascriptErrorCode::ExpectedCommaSeparatorInEnum, 1, 22, 1, 23}
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
                    {ValuascriptErrorCode::ExpectedCommaSeparatorInEnum, 1, 24, 1, 25},
                    {ValuascriptErrorCode::ExpectedCommaSeparatorInEnum, 1, 32, 1, 33}
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"A", {}, IsNumber("1")},
                              {"B", {}, IsCall(IsIdentifier("a"))},
                              {"C", {}, IsNumber("2")},
                          }
                )
            );

            reg("CasesGarbageStart", "enum Test: int { +-*/, B, C }",
                {
                    {ValuascriptErrorCode::ExpectedEnumCaseName, 1, 18, 1, 19},
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
                    {ValuascriptErrorCode::ExpectedEnumCaseName, 1, 21, 1, 22},
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
                    {ValuascriptErrorCode::ExpectedEnumCaseName, 1, 24, 1, 25},
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
                    {ValuascriptErrorCode::ExpectedEnumCaseName, 1, 21, 1, 22},
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
                    {ValuascriptErrorCode::InvalidExpression, 1, 21, 1, 22},
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"A"},
                              {"B"}
                          }
                )
            );

            reg("MissingDefaultLast", "enum Test: int { A, B = }",
                {
                    {ValuascriptErrorCode::InvalidExpression, 1, 25, 1, 26},
                },
                IsEnumDef("Test", {}, IsType("int"), {
                              {"A"},
                              {"B"}
                          }
                )
            );

            reg("MissingDefaultMultiple", "enum Test: int { A =, B = 1, C =, D = 1, E =, F = 1}",
                {
                    {ValuascriptErrorCode::InvalidExpression, 1, 21, 1, 22},
                    {ValuascriptErrorCode::InvalidExpression, 1, 33, 1, 34},
                    {ValuascriptErrorCode::InvalidExpression, 1, 45, 1, 46},
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
        const auto& [name, code, errors, verifier] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectEnumDefinitionErrors(code, errors, verifier);
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
