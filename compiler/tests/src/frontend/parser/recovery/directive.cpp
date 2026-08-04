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
                .errors = {PErr{.code = E::MissingDirectiveName, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3}},
                .verifier = IsDirective("<error>")
            });

            reg({
                .name = "MissingValueAfterEquals",
                .code = "#dir = ",
                .errors = {PErr{.code = E::MissingValueAfterEquals, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}},
                .verifier = IsDirective("dir", IsNull())
            });

            reg({
                .name = "MissingHashValuelessDirective",
                .code = "dir",
                .errors = {PErr{.code = E::InvalidStandaloneStatement, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 4}},
                .verifier = IsNull()
            });

            reg({
                .name = "MissingNamePlusValueWithoutEquals",
                .code = "# \"string\"",
                .errors = {PErr{.code = E::MissingDirectiveName, .line_start = 1, .column_start = 3, .line_end = 1, .column_end = 11}},
                .verifier = IsDirective("<error>", IsNull())
            });

            reg({
                .name = "InvalidMarkerAsteriskWithValue",
                .code = "*iterations = 1000",
                .errors = {PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 2}},
                .verifier = IsNull()
            });

            reg({
                .name = "InvalidMarkerAsteriskNoValue",
                .code = "*module",
                .errors = {PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 2}},
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
        TestNameGenerator{}
    );
}
