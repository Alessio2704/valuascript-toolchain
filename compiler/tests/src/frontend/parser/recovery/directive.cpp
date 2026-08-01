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
            auto reg = [](const RecoveryCase<DirectiveVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "MissingName",
                .code = "#",
                .errors = {{E::MissingDirectiveName, 1, 2, 1, 3}},
                .verifier = IsDirective("<error>")
            });

            reg({
                .name = "MissingValueAfterEquals",
                .code = "#dir = ",
                .errors = {{E::MissingValueAfterEquals, 1, 7, 1, 8}},
                .verifier = IsDirective("dir", IsNull())
            });

            reg({
                .name = "MissingHashValuelessDirective",
                .code = "dir",
                .errors = {{E::InvalidStandaloneStatement, 1, 1, 1, 4}},
                .verifier = IsNull()
            });

            reg({
                .name = "MissingNamePlusValueWithoutEquals",
                .code = "# \"string\"",
                .errors = {{E::MissingDirectiveName, 1, 3, 1, 11}},
                .verifier = IsDirective("<error>", IsNull())
            });

            reg({
                .name = "InvalidMarkerAsteriskWithValue",
                .code = "*iterations = 1000",
                .errors = {{E::InvalidExpression, 1, 1, 1, 2}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidMarkerAsteriskNoValue",
                .code = "*module",
                .errors = {{E::InvalidExpression, 1, 1, 1, 2}},
                .verifier = IsNull()
            });

            return true;
        }();
    }

    TEST_P(DirectiveErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectDirectiveErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
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
