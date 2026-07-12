#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_registry.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    struct MissingOperatorTestCase
    {
        std::string test_name;
        std::string snippet;
        UniversalVerifier verifier_if_binding_required;
        std::vector<UniversalVerifier> verifiers_if_multi_element;
        std::vector<ParserExpectedError> errors_if_binding_required;
    };

    class MissingOperatorExpansionRunner : public ParserTestBase,
                                           public testing::WithParamInterface<MissingOperatorTestCase>
    {
    };

    TEST_P(MissingOperatorExpansionRunner, ValidatesInAllContexts)
    {
        auto tc = GetParam();

        auto multi = std::make_shared<MultiInjectVerifier>(MultiInjectVerifier{
            tc.verifier_if_binding_required,
            tc.verifiers_if_multi_element
        });

        ExpectParseErrorsUnified(InjectableType::Expression, tc.snippet, tc.errors_if_binding_required, multi,
                                 tc.test_name);
    }

    std::vector<MissingOperatorTestCase> missing_operator_cases()
    {
        return {
            {
                .test_name = "MissingOperator1",
                .snippet = "1 2",
                .verifier_if_binding_required = IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2")),
                .verifiers_if_multi_element = {IsNumber("1"), IsNumber("2")},
                .errors_if_binding_required = {{E::MissingOperator, 1, 3, 1, 4}}
            },
            {
                .test_name = "MissingOperator2",
                .snippet = "1 a",
                .verifier_if_binding_required = IsBinary(TokenType::Error, IsNumber("1"), IsIdentifier("a")),
                .verifiers_if_multi_element = {IsNumber("1"), IsIdentifier("a")},
                .errors_if_binding_required = {{E::MissingOperator, 1, 3, 1, 4}}
            },
            {
                .test_name = "MissingOperator3",
                .snippet = "a b",
                .verifier_if_binding_required = IsBinary(TokenType::Error, IsIdentifier("a"), IsIdentifier("b")),
                .verifiers_if_multi_element = {IsIdentifier("a"), IsIdentifier("b")},
                .errors_if_binding_required = {{E::MissingOperator, 1, 3, 1, 4}}
            },
            {
                .test_name = "MissingOperator4",
                .snippet = "a 1",
                .verifier_if_binding_required = IsBinary(TokenType::Error, IsIdentifier("a"), IsNumber("1")),
                .verifiers_if_multi_element = {IsIdentifier("a"), IsNumber("1")},
                .errors_if_binding_required = {{E::MissingOperator, 1, 3, 1, 4}}
            },
            {
                .test_name = "MissingOperator5",
                .snippet = "100 + 1 2",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Plus,
                    IsNumber("100"),
                    IsBinary(TokenType::Error, IsNumber("1"), IsNumber("2"))
                ),
                .verifiers_if_multi_element = {
                    IsBinary(TokenType::Plus, IsNumber("100"), IsNumber("1")), IsNumber("2")
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 9, 1, 10}}
            },
            {
                .test_name = "MissingOperator6",
                .snippet = "1 + (2 3)",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Plus,
                    IsNumber("1"),
                    IsGrouping(IsBinary(TokenType::Error, IsNumber("2"), IsNumber("3")))
                ),
                .verifiers_if_multi_element = {
                    IsBinary(
                        TokenType::Plus,
                        IsNumber("1"),
                        IsGrouping(IsBinary(TokenType::Error, IsNumber("2"), IsNumber("3")))
                    )
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 8, 1, 9}}
            },
            {
                .test_name = "MissingOperator7",
                .snippet = "1 (2 + 3)",
                .verifier_if_binding_required = IsCall(IsNumber("1")),
                .verifiers_if_multi_element = {IsCall(IsNumber("1"))},
                .errors_if_binding_required = {{E::MissingOperator, 1, 3, 1, 4}}
            },
            {
                .test_name = "MissingOperator8",
                .snippet = "1 + a() b()",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Plus,
                    IsNumber("1"),
                    IsBinary(TokenType::Error, IsCall(IsIdentifier("a")), IsCall(IsIdentifier("b")))
                ),
                .verifiers_if_multi_element = {
                    IsBinary(TokenType::Plus, IsNumber("1"), IsCall(IsIdentifier("a"))),
                    IsCall(IsIdentifier("b"))
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 9, 1, 10}}
            },
            {
                .test_name = "MissingOperator9",
                .snippet = "1000 a() + b()",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Plus,
                    IsBinary(TokenType::Error, IsNumber("1000"), IsCall(IsIdentifier("a"))),
                    IsCall(IsIdentifier("b"))
                ),
                .verifiers_if_multi_element = {
                    IsNumber("1000"),
                    IsBinary(TokenType::Plus, IsCall(IsIdentifier("a")), IsCall(IsIdentifier("b")))
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 6, 1, 7}}
            },
            {
                .test_name = "MissingOperator10",
                .snippet = "a + b (1 + 2)",
                .verifier_if_binding_required = IsBinary(TokenType::Plus, IsIdentifier("a"), IsCall(IsIdentifier("b"))),
                .verifiers_if_multi_element = {
                    IsBinary(TokenType::Plus, IsIdentifier("a"), IsCall(IsIdentifier("b")))
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 7, 1, 8}}
            },
            {
                .test_name = "MissingOperator11",
                .snippet = "a + b model.a",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Plus,
                    IsIdentifier("a"),
                    IsBinary(TokenType::Error, IsIdentifier("b"), IsDot(IsIdentifier("model"), "a"))
                ),
                .verifiers_if_multi_element = {
                    IsBinary(TokenType::Plus, IsIdentifier("a"), IsIdentifier("b")),
                    IsDot(IsIdentifier("model"), "a")
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 7, 1, 12}}
            },
            {
                .test_name = "MissingOperator12",
                .snippet = "a + b vec[0]",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Plus,
                    IsIdentifier("a"),
                    IsBinary(TokenType::Error, IsIdentifier("b"), IsBracket(IsIdentifier("vec"), IsNumber("0")))
                ),
                .verifiers_if_multi_element = {
                    IsBinary(TokenType::Plus, IsIdentifier("a"), IsIdentifier("b")),
                    IsBracket(IsIdentifier("vec"), IsNumber("0"))
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 7, 1, 10}}
            },
            {
                .test_name = "MissingOperator13",
                .snippet = "a + b {}",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Plus,
                    IsIdentifier("a"),
                    IsBinary(TokenType::Error, IsIdentifier("b"), IsDict({}))
                ),
                .verifiers_if_multi_element = {
                    IsBinary(TokenType::Plus, IsIdentifier("a"), IsIdentifier("b")),
                    IsDict({})
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 7, 1, 8}}
            },
            {
                .test_name = "MissingOperator14",
                .snippet = "a[1] + (b.a  c)",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Plus,
                    IsBracket(IsIdentifier("a"), IsNumber("1")),
                    IsGrouping(IsBinary(TokenType::Error, IsDot(IsIdentifier("b"), "a"), IsIdentifier("c")))
                ),
                .verifiers_if_multi_element = {
                    IsBinary(
                        TokenType::Plus,
                        IsBracket(IsIdentifier("a"), IsNumber("1")),
                        IsGrouping(IsBinary(TokenType::Error, IsDot(IsIdentifier("b"), "a"), IsIdentifier("c")))
                    )
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 14, 1, 15}}
            },
            {
                .test_name = "MissingOperator15",
                .snippet = "a[1] + (b.a  c[3].b)",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Plus,
                    IsBracket(IsIdentifier("a"), IsNumber("1")),
                    IsGrouping(
                        IsBinary(
                            TokenType::Error,
                            IsDot(IsIdentifier("b"), "a"),
                            IsDot(IsBracket(IsIdentifier("c"), IsNumber("3")), "b")
                        )
                    )
                ),
                .verifiers_if_multi_element = {
                    IsBinary(
                        TokenType::Plus,
                        IsBracket(IsIdentifier("a"), IsNumber("1")),
                        IsGrouping(
                            IsBinary(
                                TokenType::Error,
                                IsDot(IsIdentifier("b"), "a"),
                                IsDot(IsBracket(IsIdentifier("c"), IsNumber("3")), "b")
                            )
                        )
                    )
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 14, 1, 15}}
            },
            {
                .test_name = "MissingOperator16",
                .snippet = "a + a.key (1 + 2)",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Plus,
                    IsIdentifier("a"),
                    IsCall(IsDot(IsIdentifier("a"), "key"))
                ),
                .verifiers_if_multi_element = {
                    IsBinary(
                        TokenType::Plus,
                        IsIdentifier("a"),
                        IsCall(IsDot(IsIdentifier("a"), "key"))
                    )
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 11, 1, 12}}
            },
            {
                .test_name = "MissingOperator17",
                .snippet = "1 + a[0] + b[1:2] a.b",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Plus,
                    IsBinary(TokenType::Plus, IsNumber("1"), IsBracket(IsIdentifier("a"), IsNumber("0"))),
                    IsBinary(TokenType::Error,
                             IsBracket(IsIdentifier("b"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))),
                             IsDot(IsIdentifier("a"), "b"))
                ),
                .verifiers_if_multi_element = {
                    IsBinary(
                        TokenType::Plus,
                        IsBinary(TokenType::Plus, IsNumber("1"), IsBracket(IsIdentifier("a"), IsNumber("0"))),
                        IsBracket(IsIdentifier("b"), IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2")))
                    ),
                    IsDot(IsIdentifier("a"), "b")
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 19, 1, 20}}
            },
            {
                .test_name = "MissingOperator18",
                .snippet = "1 + a() (2 + 3)",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Plus,
                    IsNumber("1"),
                    IsCall(IsCall(IsIdentifier("a")))
                ),
                .verifiers_if_multi_element = {
                    IsBinary(TokenType::Plus, IsNumber("1"), IsCall(IsCall(IsIdentifier("a"))))
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 9, 1, 10}}
            },
            {
                .test_name = "MissingOperator19",
                .snippet = "a[1]  (b - c)",
                .verifier_if_binding_required = IsCall(IsBracket(IsIdentifier("a"), IsNumber("1"))),
                .verifiers_if_multi_element = {
                    IsCall(IsBracket(IsIdentifier("a"), IsNumber("1")))
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 7, 1, 8}}
            },
            {
                .test_name = "MissingOperator20",
                .snippet = "a[1] / (b  c)",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Slash,
                    IsBracket(IsIdentifier("a"), IsNumber("1")),
                    IsGrouping(IsBinary(TokenType::Error, IsIdentifier("b"), IsIdentifier("c")))
                ),
                .verifiers_if_multi_element = {
                    IsBinary(
                        TokenType::Slash,
                        IsBracket(IsIdentifier("a"), IsNumber("1")),
                        IsGrouping(IsBinary(TokenType::Error, IsIdentifier("b"), IsIdentifier("c")))
                    )
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 12, 1, 13}}
            },
            {
                .test_name = "MissingOperator21",
                .snippet = "a[1] / (1  c)",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Slash,
                    IsBracket(IsIdentifier("a"), IsNumber("1")),
                    IsGrouping(IsBinary(TokenType::Error, IsNumber("1"), IsIdentifier("c")))
                ),
                .verifiers_if_multi_element = {
                    IsBinary(
                        TokenType::Slash,
                        IsBracket(IsIdentifier("a"), IsNumber("1")),
                        IsGrouping(IsBinary(TokenType::Error, IsNumber("1"), IsIdentifier("c")))
                    )
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 12, 1, 13}}
            },
            {
                .test_name = "MissingOperator22",
                .snippet = "a[1] (1 + c)",
                .verifier_if_binding_required = IsCall(IsBracket(IsIdentifier("a"), IsNumber("1"))),
                .verifiers_if_multi_element = {
                    IsCall(IsBracket(IsIdentifier("a"), IsNumber("1")))
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 6, 1, 7}}
            },
            {
                .test_name = "MissingOperator23",
                .snippet = "a ([1, 2])",
                .verifier_if_binding_required = IsCall(IsIdentifier("a")),
                .verifiers_if_multi_element = {
                    IsCall(IsIdentifier("a"))
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 3, 1, 4}}
            },
            {
                .test_name = "MissingOperator24",
                .snippet = "a ({1, 2})",
                .verifier_if_binding_required = IsCall(IsIdentifier("a")),
                .verifiers_if_multi_element = {
                    IsCall(IsIdentifier("a"))
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 3, 1, 4}}
            },
            {
                .test_name = "MissingOperator25",
                .snippet = "a ([[1, 2], [3, 4]])",
                .verifier_if_binding_required = IsCall(IsIdentifier("a")),
                .verifiers_if_multi_element = {
                    IsCall(IsIdentifier("a"))
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 3, 1, 4}}
            },
            {
                .test_name = "MissingOperator26",
                .snippet = "a (-5)",
                .verifier_if_binding_required = IsCall(IsIdentifier("a")),
                .verifiers_if_multi_element = {
                    IsCall(IsIdentifier("a"))
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 3, 1, 4}}
            },
            {
                .test_name = "MissingOperator27",
                .snippet = "13_624 / 11%   4",
                .verifier_if_binding_required = IsBinary(
                    TokenType::Slash, IsNumber("13_624"),
                    IsBinary(TokenType::Error, IsPercentage("11%"), IsNumber("4"))
                ),
                .verifiers_if_multi_element = {
                    IsBinary(TokenType::Slash, IsNumber("13_624"), IsPercentage("11%")),
                    IsNumber("4")
                },
                .errors_if_binding_required = {{E::MissingOperator, 1, 16, 1, 17}}
            }
        };
    }

    INSTANTIATE_TEST_SUITE_P(
        Expression,
        MissingOperatorExpansionRunner,
        testing::ValuesIn(missing_operator_cases()),
        [](const testing::TestParamInfo<MissingOperatorTestCase>& param_info) {
        return param_info.param.test_name;
        }
    );
}
