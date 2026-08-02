#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_registry.h"
#include "missing_operator_shared.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    class MissingOperatorTwoLeavesRunner : public ParserTestBase,
                                           public testing::WithParamInterface<TwoLeavesPairDef>
    {
    };

    TEST_P(MissingOperatorTwoLeavesRunner, ValidatesInAllContexts)
    {
        auto tc = GetParam();
        const auto& a = tc.a;
        const auto& b = tc.b;

        std::string snippet = a.code + " " + b.code;
        size_t start_col = a.code.length() + 2;
        size_t end_col = start_col + b.first_token_len;

        std::vector<ParserExpectedError> errs = {PErr{.code = E::MissingOperator, .line_start = 1, .column_start = start_col, .line_end = 1, .column_end = end_col}};
        auto req = IsBinary(TokenType::Error, a.verifier, b.verifier);
        std::vector<UniversalVerifier> multi = {a.verifier, b.verifier};

        auto m_v = std::make_shared<MultiInjectVerifier>(MultiInjectVerifier{req, multi});
        ExpectParseErrorsUnified(InjectableType::Expression, snippet, errs, m_v,
                                 "TwoLeaves_" + a.name + "_" + b.name);
    }

    INSTANTIATE_TEST_SUITE_P(
        TwoLeaves,
        MissingOperatorTwoLeavesRunner,
        testing::ValuesIn(get_two_leaves_pairs()),
        [](const testing::TestParamInfo<TwoLeavesPairDef>& param_info) {
            return param_info.param.test_name;
        }
    );

    class MissingOperatorExpansionRunner : public ParserTestBase,
                                           public testing::WithParamInterface<MissingOperatorExpansionDef>
    {
    };

    TEST_P(MissingOperatorExpansionRunner, ValidatesInAllContexts)
    {
        auto tc = GetParam();
        const auto& atoms = get_atoms();

        if (tc.type == TemplateType::ThreeLeaves)
        {
            const auto& a = atoms[0];
            const auto& b = atoms[1];
            const auto& c = atoms[2];

            if (tc.position == ExpansionPosition::Pos1)
            {
                std::string snippet = a.code + " " + b.code + " + " + c.code;
                size_t start_col = a.code.length() + 2;
                size_t end_col = start_col + b.first_token_len;

                std::vector<ParserExpectedError> errs = {PErr{.code = E::MissingOperator, .line_start = 1, .column_start = start_col, .line_end = 1, .column_end = end_col}};
                auto req = IsBinary(TokenType::Plus, IsBinary(TokenType::Error, a.verifier, b.verifier),
                                    c.verifier);
                std::vector<UniversalVerifier> multi = {
                    a.verifier, IsBinary(TokenType::Plus, b.verifier, c.verifier)
                };

                auto m_v = std::make_shared<MultiInjectVerifier>(MultiInjectVerifier{req, multi});
                ExpectParseErrorsUnified(InjectableType::Expression, snippet, errs, m_v,
                                         tc.test_name + "_" + a.name + "_" + b.name + "_" + c.name);
            }
            else if (tc.position == ExpansionPosition::Pos2)
            {
                std::string snippet = a.code + " + " + b.code + " " + c.code;
                size_t start_col = a.code.length() + 3 + b.code.length() + 2;
                size_t end_col = start_col + c.first_token_len;

                std::vector<ParserExpectedError> errs = {PErr{.code = E::MissingOperator, .line_start = 1, .column_start = start_col, .line_end = 1, .column_end = end_col}};
                auto req = IsBinary(TokenType::Plus, a.verifier,
                                    IsBinary(TokenType::Error, b.verifier, c.verifier));
                std::vector<UniversalVerifier> multi = {
                    IsBinary(TokenType::Plus, a.verifier, b.verifier), c.verifier
                };

                auto m_v = std::make_shared<MultiInjectVerifier>(MultiInjectVerifier{req, multi});
                ExpectParseErrorsUnified(InjectableType::Expression, snippet, errs, m_v,
                                         tc.test_name + "_" + a.name + "_" + b.name + "_" + c.name);
            }
        }
        else if (tc.type == TemplateType::FourLeaves)
        {
            const auto& a = atoms[0];
            const auto& b = atoms[1];
            const auto& c = atoms[2];
            const auto& d = atoms[3];

            if (tc.position == ExpansionPosition::Pos1)
            {
                std::string snippet = a.code + " " + b.code + " + " + c.code + " + " + d.code;
                size_t start_col = a.code.length() + 2;
                size_t end_col = start_col + b.first_token_len;

                std::vector<ParserExpectedError> errs = {
                    PErr{.code = E::MissingOperator, .line_start = 1, .column_start = start_col, .line_end = 1, .column_end = end_col}
                };
                auto req = IsBinary(TokenType::Plus,
                                    IsBinary(TokenType::Plus,
                                             IsBinary(TokenType::Error, a.verifier, b.verifier),
                                             c.verifier), d.verifier);
                std::vector<UniversalVerifier> multi = {
                    a.verifier,
                    IsBinary(TokenType::Plus, IsBinary(TokenType::Plus, b.verifier, c.verifier),
                             d.verifier)
                };

                auto m_v = std::make_shared<MultiInjectVerifier>(MultiInjectVerifier{req, multi});
                ExpectParseErrorsUnified(InjectableType::Expression, snippet, errs, m_v,
                                         tc.test_name + "_" + a.name + "_" + b.name + "_" + c.name + "_" + d.name);
            }
            else if (tc.position == ExpansionPosition::Pos2)
            {
                std::string snippet = a.code + " + " + b.code + " " + c.code + " + " + d.code;
                size_t start_col = a.code.length() + 3 + b.code.length() + 2;
                size_t end_col = start_col + c.first_token_len;

                std::vector<ParserExpectedError> errs = {
                    PErr{.code = E::MissingOperator, .line_start = 1, .column_start = start_col, .line_end = 1, .column_end = end_col}
                };
                auto req = IsBinary(TokenType::Plus,
                                    IsBinary(TokenType::Plus, a.verifier,
                                             IsBinary(TokenType::Error, b.verifier, c.verifier)),
                                    d.verifier);
                std::vector<UniversalVerifier> multi = {
                    IsBinary(TokenType::Plus, a.verifier, b.verifier),
                    IsBinary(TokenType::Plus, c.verifier, d.verifier)
                };

                auto m_v = std::make_shared<MultiInjectVerifier>(MultiInjectVerifier{req, multi});
                ExpectParseErrorsUnified(InjectableType::Expression, snippet, errs, m_v,
                                         tc.test_name + "_" + a.name + "_" + b.name + "_" + c.name + "_" + d.name);
            }
            else if (tc.position == ExpansionPosition::Pos3)
            {
                std::string snippet = a.code + " + " + b.code + " + " + c.code + " " + d.code;
                size_t start_col = a.code.length() + 3 + b.code.length() + 3 + c.code.length() + 2;
                size_t end_col = start_col + d.first_token_len;

                std::vector<ParserExpectedError> errs = {
                    PErr{.code = E::MissingOperator, .line_start = 1, .column_start = start_col, .line_end = 1, .column_end = end_col}
                };
                auto req = IsBinary(TokenType::Plus, IsBinary(TokenType::Plus, a.verifier, b.verifier),
                                    IsBinary(TokenType::Error, c.verifier, d.verifier));
                std::vector<UniversalVerifier> multi = {
                    IsBinary(TokenType::Plus, IsBinary(TokenType::Plus, a.verifier, b.verifier),
                             c.verifier),
                    d.verifier
                };

                auto m_v = std::make_shared<MultiInjectVerifier>(MultiInjectVerifier{req, multi});
                ExpectParseErrorsUnified(InjectableType::Expression, snippet, errs, m_v,
                                         tc.test_name + "_" + a.name + "_" + b.name + "_" + c.name + "_" + d.name);
            }
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ExpressionTemplates,
        MissingOperatorExpansionRunner,
        testing::ValuesIn(get_expansion_cases()),
        [](const testing::TestParamInfo<MissingOperatorExpansionDef>& param_info) {
            return param_info.param.test_name;
        }
    );

    class MissingOperatorSpecialCasesRunner : public ParserTestBase,
                                              public testing::WithParamInterface<SpecialCaseDef>
    {
    };

    TEST_P(MissingOperatorSpecialCasesRunner, ValidatesInAllContexts)
    {
        const auto& special_case = GetParam();
        std::vector<ParserExpectedError> errs = {PErr{.code = E::MissingOperator, .line_start = 1, .column_start = special_case.start_col, .line_end = 1, .column_end = special_case.end_col}};
        auto m_v = std::make_shared<MultiInjectVerifier>(MultiInjectVerifier{special_case.verifier, special_case.multi});
        ExpectParseErrorsUnified(InjectableType::Expression, special_case.snippet, errs, m_v,
                                 "SpecialCases_" + special_case.test_name);
    }

    INSTANTIATE_TEST_SUITE_P(
        SpecialCases,
        MissingOperatorSpecialCasesRunner,
        testing::ValuesIn(get_special_cases()),
        [](const testing::TestParamInfo<SpecialCaseDef>& param_info) {
            return param_info.param.test_name;
        }
    );
}
