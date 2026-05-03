#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/error_registry.h"
#include "frontend/parser/helpers/recovery_sentinel.h"

namespace valuascript::compiler::test
{
    class ExpressionsErrorRegistryRunner : public ParserTestBase,
                                         public testing::WithParamInterface<ErrorRegistryEntry<ExprVerifier>>
    {
    };

    TEST_P(ExpressionsErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectExpressionErrors(code, errors, verifier);
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
