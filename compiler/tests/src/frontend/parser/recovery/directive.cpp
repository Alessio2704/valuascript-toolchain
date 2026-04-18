#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class DirectiveErrorTest : public ParserTestBase
    {
    };

    TEST_F(DirectiveErrorTest, MissingName)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingDirectiveName, 1, 2, 1, 3);

        ExpectParseErrorsWithRecovery(
            "#",
            errors,
            ProgramSpec{
                .directives = {
                    IsDirective("<error>")
                }
            }
        );
    }

    TEST_F(DirectiveErrorTest, MissingValueAfterEquals)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingValueAfterEquals, 1, 7, 1, 8);

        ExpectParseErrorsWithRecovery(
            "#dir = ",
            errors,
            ProgramSpec{
                .directives = {
                    IsDirective("dir", IsNull())
                }
            }
        );
    }

    TEST_F(DirectiveErrorTest, MissingHashValuelessDirective)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::InvalidStandaloneStatement, 1, 1, 1, 4);

        ExpectParseErrorsWithRecovery(
            "dir",
            errors,
            ProgramSpec{
                .directives = {}
            }
        );
    }

    TEST_F(DirectiveErrorTest, MissingNamePlusValueWithoutEquals)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingDirectiveName, 1, 3, 1, 11);

        ExpectParseErrorsWithRecovery(
            "# \"string\"",
            errors,
            ProgramSpec{
                .directives = {
                    IsDirective("<error>", IsNull())
                }
            }
        );
    }

    TEST_F(DirectiveErrorTest, InvalidMarkerAsteriskWithValue)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::InvalidExpression, 1, 1, 1, 2);

        ExpectParseErrorsWithRecovery(
            "*iterations = 1000",
            errors,
            ProgramSpec{
                .directives = {}
            }
        );
    }

    TEST_F(DirectiveErrorTest, InvalidMarkerAsteriskNoValue)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::InvalidExpression, 1, 1, 1, 2);

        ExpectParseErrorsWithRecovery(
            "*module",
            errors,
            ProgramSpec{
                .directives = {}
            }
        );
    }
}
