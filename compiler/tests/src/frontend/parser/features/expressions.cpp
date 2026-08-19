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
        const auto& entry = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + entry.test_name);

        ExpectValidExpression(entry.code, entry.verifier, entry.skip_contexts);
    }

    INSTANTIATE_TEST_SUITE_P(
        Expression,
        ExpressionRegistryRunner,
        testing::ValuesIn(ConstructRegistry::expressions()),
        TestNameGenerator{}
    );
}
