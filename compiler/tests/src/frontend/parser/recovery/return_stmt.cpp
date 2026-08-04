#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ReturnStatementErrorRegistryRunner : public ParserTestBase,
                                               public testing::WithParamInterface<ErrorRegistryEntry<ReturnVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<ReturnVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "ReturnInvalidExpression",
                .code = "return *",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9
                    }
                },
                .verifier = IsReturn({}, {IsNull()})
            });

            return true;
        }();
    }

    TEST_P(ReturnStatementErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels,
            accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectReturnErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels,
                           accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        ReturnStatement,
        ReturnStatementErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::returns()),
        TestNameGenerator{}
    );
}
