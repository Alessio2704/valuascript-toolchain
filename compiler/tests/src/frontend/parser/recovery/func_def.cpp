#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class FuncDefErrorRegistryRunner : public ParserTestBase,
                                       public testing::WithParamInterface<ErrorRegistryEntry<FuncVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs,
                          const OneOf<FuncVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("MissingTypeAfterArrowDiscardsAndContinues", "func test() -> , int {}",
                {{E::MissingTypeAnnotation, 1, 16, 1, 17}},
                IsFunctionDef("test", {}, {}, {
                                  IsNullType(),
                                  IsType("int")
                              }
                )
            );


            return true;
        }();
    }

    TEST_P(FuncDefErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectFunctionDefinitionErrors(code, errors, verifier, skip_contexts);
    }

    INSTANTIATE_TEST_SUITE_P(
        FunctionDefinition,
        FuncDefErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::functions()),
        [](const testing::TestParamInfo<ErrorRegistryEntry<FuncVerifier>>& test_info)
        {
        return test_info.param.test_name;
        }
    );
}
