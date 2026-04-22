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
            auto reg = [](auto n, auto c, const auto& v) { ConstructRegistry::add(n, c, v); };

            reg("ReturnSingleValue",
                "return 1",
                IsReturn({IsNumber("1")}));

            reg("ReturnMultipleValues",
                "return 1, true, \"success\"",
                IsReturn({
                    IsNumber("1"),
                    IsBoolean(true),
                    IsString("\"success\"")
                }));

            reg("ReturnWithIdentifier",
                "return result",
                IsReturn({IsIdentifier("result")}));

            reg("MultilineFormatting",
                "return \n"
                "  1, \n"
                "  2",
                IsReturn({
                    IsNumber("1"),
                    IsNumber("2")
                }));

            return true;
        }();
    }

    TEST_P(ReturnStatementRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidReturn(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        ReturnStatement,
        ReturnStatementRegistryRunner,
        testing::ValuesIn(ConstructRegistry::returns()),
        [](const testing::TestParamInfo<RegistryEntry<ReturnVerifier>>& info) {
        return info.param.test_name;
        }
    );
}
