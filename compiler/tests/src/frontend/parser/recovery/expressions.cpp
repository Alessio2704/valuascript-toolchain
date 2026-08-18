#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ExpressionsErrorRegistryRunner : public ParserTestBase,
                                         public testing::WithParamInterface<ErrorRegistryEntry<ExprVerifier>>
    {
    };

    TEST_P(ExpressionsErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& p = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + p.test_name);

        ExpectExpressionErrors(p.code, p.errors, p.verifier, p.skip_contexts, p.context_overrides, p.excluded_sentinels, p.accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        Expression,
        ExpressionsErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::expressions()),
        TestNameGenerator{}
    );
}
