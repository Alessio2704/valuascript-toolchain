#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class ExpressionRegistryRunner : public ParserTestBase,
                                     public testing::WithParamInterface<RegistryEntry<ExprVerifier>>
    {
    };

    TEST_P(ExpressionRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidExpression(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        Expression,
        ExpressionRegistryRunner,
        testing::ValuesIn(ConstructRegistry::expressions()),
        [](const testing::TestParamInfo<RegistryEntry<ExprVerifier>>& info)
        {
            return info.param.test_name;
        }
    );
}
