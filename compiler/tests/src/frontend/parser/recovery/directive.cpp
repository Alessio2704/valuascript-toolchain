#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/error_registry.h"
#include "frontend/parser/helpers/recovery_sentinel.h"

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
            auto reg = [](auto n, auto c, const auto& errs, const auto& v) { ErrorRegistry::add(n, c, errs, v); };

            reg("MissingName", "#",
                std::vector<ExpectedError>{{ValuascriptErrorCode::MissingDirectiveName, 1, 2, 1, 3}},
                IsDirective("<error>"));

            reg("MissingValueAfterEquals", "#dir = ",
                std::vector<ExpectedError>{{ValuascriptErrorCode::MissingValueAfterEquals, 1, 7, 1, 8}},
                IsDirective("dir", IsNull()));

            reg("MissingHashValuelessDirective", "dir",
                std::vector<ExpectedError>{{ValuascriptErrorCode::InvalidStandaloneStatement, 1, 1, 1, 4}},
                IsNull());

            reg("MissingNamePlusValueWithoutEquals", "# \"string\"",
                std::vector<ExpectedError>{{ValuascriptErrorCode::MissingDirectiveName, 1, 3, 1, 11}},
                IsDirective("<error>", IsNull()));

            reg("InvalidMarkerAsteriskWithValue", "*iterations = 1000",
                std::vector<ExpectedError>{{ValuascriptErrorCode::InvalidExpression, 1, 1, 1, 2}},
                IsNull());

            reg("InvalidMarkerAsteriskNoValue", "*module",
                std::vector<ExpectedError>{{ValuascriptErrorCode::InvalidExpression, 1, 1, 1, 2}},
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
        [](const testing::TestParamInfo<ErrorRegistryEntry<DirectiveVerifier>>& info) {
        return info.param.test_name;
        }
    );
}
