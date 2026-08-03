#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class ReturnStatementRegistryRunner : public ParserTestBase,
                                          public testing::WithParamInterface<RegistryEntry<ReturnVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<ReturnVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "ReturnSingleValue",
                .code = "return 1",
                .verifier = IsReturn({}, {IsNumber("1")})
            });

            reg({
                .name = "ReturnMultipleValues",
                .code = "return 1, true, \"success\"",
                .verifier = IsReturn({}, {
                    IsNumber("1"),
                    IsBoolean(true),
                    IsString("\"success\"")
                })
            });

            reg({
                .name = "ReturnWithIdentifier",
                .code = "return result",
                .verifier = IsReturn({}, {IsIdentifier("result")})
            });

            reg({
                .name = "MultilineFormatting",
                .code = "return \n"
                "  1, \n"
                "  2",
                .verifier = IsReturn({}, {
                    IsNumber("1"),
                    IsNumber("2")
                })
            });

            return true;
        }();
    }

    TEST_P(ReturnStatementRegistryRunner, ValidatesInAllContexts)
    {
        const auto& entry = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + entry.test_name);

        ExpectValidReturn(entry.code, entry.verifier, entry.skip_contexts);
    }

    INSTANTIATE_TEST_SUITE_P(
        ReturnStatement,
        ReturnStatementRegistryRunner,
        testing::ValuesIn(ConstructRegistry::returns()),
        [](const testing::TestParamInfo<RegistryEntry<ReturnVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
