#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test {
    class DecimalMissingLeadingZeroTest : public ParserTestBase {
    };

    TEST_F(DecimalMissingLeadingZeroTest, PartialAST) {
        ExpectParseErrorsWithRecovery(
            "let a = .5",
            {
                {ValuascriptErrorCode::DecimalMissingLeadingZero, 1, 9, 1, 10}
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsNumber(".5"))
                }
            }
        );
    }

    TEST_F(DecimalMissingLeadingZeroTest, PercentageLiteralPartialAST) {
        ExpectParseErrorsWithRecovery(
            "let a = .5%",
            {
                {ValuascriptErrorCode::DecimalMissingLeadingZero, 1, 9, 1, 10}
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsPercentage(".5%"))
                }
            }
        );
    }
}
