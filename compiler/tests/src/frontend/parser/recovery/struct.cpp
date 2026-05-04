#include "../helpers/node_matchers.h"
#include "frontend/parser/errors_synchronization/parser_errors_synchronization_base.h"
#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/error_registry.h"
#include "frontend/parser/helpers/recovery_sentinel.h"

namespace valuascript::compiler::test
{
    class StructErrorRegistryRunner : public ParserTestBase,
                                      public testing::WithParamInterface<ErrorRegistryEntry<StructVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<StructVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("MissingStructName", "struct { id: int }",
                {{ValuascriptErrorCode::ExpectedStructName, 1, 8, 1, 9}},
                IsStructDef("<error>", {}, {
                                {"id", {}, IsType("int")}
                            }
                )
            );

            reg("MissingStructLeftBrace", "struct Test id: int }",
                {{ValuascriptErrorCode::ExpectedBraceInStructDefinition, 1, 13, 1, 14}},
                IsNull()
            );

            return true;
        }();
    }

    TEST_P(StructErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectStructDefinitionErrors(code, errors, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        Struct,
        StructErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::structs()),
        [](const testing::TestParamInfo<ErrorRegistryEntry<StructVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
