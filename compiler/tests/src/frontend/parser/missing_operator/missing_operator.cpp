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
