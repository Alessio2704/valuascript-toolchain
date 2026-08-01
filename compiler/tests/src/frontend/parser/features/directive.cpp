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
            auto reg = [](const ConstructCase<DirectiveVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "NoEqual1",
                .code = "#no_equal 1",
                .verifier = IsDirective("no_equal", IsNumber("1"))
            });

            reg({
                .name = "NoValueDirective",
                .code = "#no_value",
                .verifier = IsDirective("no_value", IsNull())
            });

            reg({
                .name = "NoValueDirectiveUnderscoreAndNumber",
                .code = "#no_value_1",
                .verifier = IsDirective("no_value_1", IsNull())
            });

            reg({
                .name = "ValueDirective1",
                .code = "#value = 1",
                .verifier = IsDirective("value", IsNumber("1"))
            });

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
