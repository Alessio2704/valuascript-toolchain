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

        ExpectParseErrorsUnified(InjectableType::Expression, tc.snippet, tc.errors_if_binding_required, multi, tc.test_name);
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
