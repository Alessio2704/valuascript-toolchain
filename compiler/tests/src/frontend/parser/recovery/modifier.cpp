#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ModifierErrorRegistryRunner : public ParserTestBase,
                                          public testing::WithParamInterface<ErrorRegistryEntry<ModifierVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs,
                          const OneOf<ModifierVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("ModifierMissingColon", "@test(a 1, b: 2)",
                {{E::MissingColonAfterArgument, 1, 9, 1, 10}},
                std::vector<ModifierSpec>{
                    {
                        "test", {
                            {"<error>", IsNull()},
                            {"b", IsNumber("2")},
                        }
                    }
                }
            );

            return true;
        }();
    }

    TEST_P(ModifierErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectModifierErrors(code, errors, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        Modifier,
        ModifierErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::modifiers()),
        [](const testing::TestParamInfo<ErrorRegistryEntry<ModifierVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
