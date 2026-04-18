#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class DecimalMissingLeadingZeroTest : public ParserTestBase
    {
    };

    TEST_F(DecimalMissingLeadingZeroTest, PartialAST)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::DecimalMissingLeadingZero, 1, 9, 1, 10);

        ExpectParseErrorsWithRecovery(
            "let a = .5",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsNumber(".5"))
                }
            }
        );
    }

    TEST_F(DecimalMissingLeadingZeroTest, PercentageLiteralPartialAST)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::DecimalMissingLeadingZero, 1, 9, 1, 10);

        ExpectParseErrorsWithRecovery(
            "let a = .5%",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsPercentage(".5%"))
                }
            }
        );
    }
}
