#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_names_helpers.h"

namespace valuascript::compiler::test
{
    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<ExprVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "GroupingMissingClosingParen",
                .code = "( 1 + 2",
                .errors = {
                    PErr{
                        .code = E::ExpectedRightParenAfterExpression,
                        .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8
                    }
                },
                .verifier = IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2"))),
                .skip_contexts = ContextNames::all_nested_swallowing_grouping_contexts(),
                .accepted_sentinels = SentinelKinds::all()
            });

            return true;
        }();
    }
}
