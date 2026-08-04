#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"
#include "token/reserved_keyword_lookup.h"
#include "frontend/parser/helpers/recovery_sentinel.h"
#include "frontend/parser/helpers/error_shifter.h"

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

        ExpectParseErrorsUnified(
            InjectableType::Identifier,
            keyword,
            {base_error},
            UniversalVerifier(keyword),
            "KeywordTest"
        );
    }

    INSTANTIATE_TEST_SUITE_P(
        ExhaustiveKeywords,
        ReservedKeywordRecoveryTest,
        testing::ValuesIn(shared::get_all_reserved_keyword_strings()),
        TestNameGenerator{}
    );
}
