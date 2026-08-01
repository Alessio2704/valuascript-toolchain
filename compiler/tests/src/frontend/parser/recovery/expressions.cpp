#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ExpressionsErrorRegistryRunner : public ParserTestBase,
                                         public testing::WithParamInterface<ErrorRegistryEntry<ExprVerifier>>
    {
    };

    TEST_P(ExpressionsErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectExpressionErrors(code, errors, verifier, skip_contexts, context_overrides);
    }

    INSTANTIATE_TEST_SUITE_P(
        Expression,
        ExpressionsErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::expressions()),
        [](const testing::TestParamInfo<ErrorRegistryEntry<ExprVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
