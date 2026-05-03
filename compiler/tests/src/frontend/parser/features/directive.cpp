#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class DirectiveRegistryRunner : public ParserTestBase,
                                    public testing::WithParamInterface<RegistryEntry<DirectiveVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const auto& v) { ConstructRegistry::add(n, c, v); };

            reg("NoEqual1",
                "#no_equal 1",
                IsDirective("no_equal", IsNumber("1")));

            reg("NoValueDirective",
                "#no_value",
                IsDirective("no_value", IsNull()));

            reg("NoValueDirectiveUnderscoreAndNumber",
                "#no_value_1",
                IsDirective("no_value_1", IsNull()));

            reg("ValueDirective1",
                "#value = 1",
                IsDirective("value", IsNumber("1")));

            return true;
        }();
    }

    TEST_P(DirectiveRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidDirective(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        Directive,
        DirectiveRegistryRunner,
        testing::ValuesIn(ConstructRegistry::directives()),
        [](const testing::TestParamInfo<RegistryEntry<DirectiveVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
