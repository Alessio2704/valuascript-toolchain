#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class DirectiveErrorRegistryRunner : public ParserTestBase,
                                         public testing::WithParamInterface<ErrorRegistryEntry<DirectiveVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs,
                          const OneOf<DirectiveVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("MissingName", "#",
                {{E::MissingDirectiveName, 1, 2, 1, 3}},
                IsDirective("<error>")
            );

            reg("MissingValueAfterEquals", "#dir = ",
                {{E::MissingValueAfterEquals, 1, 7, 1, 8}},
                IsDirective("dir", IsNull())
            );

            reg("MissingHashValuelessDirective", "dir",
                {{E::InvalidStandaloneStatement, 1, 1, 1, 4}},
                IsNull()
            );

            reg("MissingNamePlusValueWithoutEquals", "# \"string\"",
                {{E::MissingDirectiveName, 1, 3, 1, 11}},
                IsDirective("<error>", IsNull())
            );

            reg("InvalidMarkerAsteriskWithValue", "*iterations = 1000",
                {{E::InvalidExpression, 1, 1, 1, 2}},
                IsNull()
            );

            reg("InvalidMarkerAsteriskNoValue", "*module",
                {{E::InvalidExpression, 1, 1, 1, 2}},
                IsNull()
            );

            return true;
        }();
    }

    TEST_P(DirectiveErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectDirectiveErrors(code, errors, verifier, skip_contexts);
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
