#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class DirectiveErrorRegistryRunner : public ParserTestBase,
                                         public testing::WithParamInterface<ErrorRegistryEntry<DirectiveVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<DirectiveVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("MissingName", "#",
                {{ValuascriptErrorCode::MissingDirectiveName, 1, 2, 1, 3}},
                IsDirective("<error>"));

            reg("MissingValueAfterEquals", "#dir = ",
                {{ValuascriptErrorCode::MissingValueAfterEquals, 1, 7, 1, 8}},
                IsDirective("dir", IsNull()));

            reg("MissingHashValuelessDirective", "dir",
                {{ValuascriptErrorCode::InvalidStandaloneStatement, 1, 1, 1, 4}},
                IsNull());

            reg("MissingNamePlusValueWithoutEquals", "# \"string\"",
                {{ValuascriptErrorCode::MissingDirectiveName, 1, 3, 1, 11}},
                IsDirective("<error>", IsNull()));

            reg("InvalidMarkerAsteriskWithValue", "*iterations = 1000",
                {{ValuascriptErrorCode::InvalidExpression, 1, 1, 1, 2}},
                IsNull());

            reg("InvalidMarkerAsteriskNoValue", "*module",
                {{ValuascriptErrorCode::InvalidExpression, 1, 1, 1, 2}},
                IsNull());

            return true;
        }();
    }

    TEST_P(DirectiveErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectDirectiveErrors(code, errors, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        Directive,
        DirectiveErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::directives()),
        [](const testing::TestParamInfo<ErrorRegistryEntry<DirectiveVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
