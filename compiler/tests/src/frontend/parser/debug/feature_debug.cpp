#include "expansion_debug_helper.h"
#include "utils/parametrised_test_name_helper.h"

namespace valuascript::compiler::test
{
    class ExpansionFeatureDebugger : public ExpansionDebugHelper,
                                     public testing::WithParamInterface<DumpTest>
    {
    };

#if defined(ENABLE_DEBUG_DUMPS) && ENABLE_DEBUG_DUMPS
    TEST_P(ExpansionFeatureDebugger, GenerateFeatureDump)
    {
        DebugFeature(GetParam().name);
    }

    INSTANTIATE_TEST_SUITE_P(
        FeatureDumps,
        ExpansionFeatureDebugger,
        testing::Values(
            DumpTest{.name = "IntegerLiteral"},
            DumpTest{.name = "Generic"},
            DumpTest{.name = "SingleModifier"},
            DumpTest{.name = "ReturnSingleValue"},
            DumpTest{.name = "ExplicitType"}
        ),
        TestNameGenerator{}
    );
#endif
}

