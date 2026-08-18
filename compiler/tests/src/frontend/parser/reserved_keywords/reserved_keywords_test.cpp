#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_names.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler::test
{
    class ReservedKeywordRecoveryTest : public ParserTestBase,
                                        public testing::WithParamInterface<std::string>
    {
    };

    TEST_P(ReservedKeywordRecoveryTest, KeywordRejectedAndRecoveredExhaustively)
    {
        std::string keyword = GetParam();

        ParserExpectedError base_error(ParserErrorCode::ReservedKeywordAsIdentifier, 1, 1, 1, keyword.length() + 1);

        std::vector<std::string_view> skip_contexts = {};
        if (keyword == "true" || keyword == "false" || keyword == "self" ||
            keyword == "if" || keyword == "switch" || keyword == "not" ||
            keyword == "else" || keyword == "then" || keyword == "case" || keyword == "default" || keyword == "return")
        {
            skip_contexts.push_back(ContextNames::IdAsExpression);
        }

        ExpectParseErrorsUnified(
            InjectableType::Identifier,
            keyword,
            {base_error},
            UniversalVerifier(keyword),
            "KeywordTest",
            skip_contexts
        );
    }

    INSTANTIATE_TEST_SUITE_P(
        ExhaustiveKeywords,
        ReservedKeywordRecoveryTest,
        testing::ValuesIn(shared::get_all_reserved_keyword_strings()),
        TestNameGenerator{}
    );
}
