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
}