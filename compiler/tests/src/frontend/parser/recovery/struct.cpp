#include "frontend/parser/helpers/parser_test_base.h"

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

            // reg("OnlyStruct", "struct",
            //     {{ValuascriptErrorCode::ExpectedStructName, 1, 8, 1, 9}},
            //     IsStructDef("<error>", {}, {})
            // );

            // reg("OnlyStructName", "struct Test",
            //     {{ValuascriptErrorCode::ExpectedBraceInStructDefinition, 1, 12, 1, 13}},
            //     IsStructDef("Test", {}, {})
            // );

            reg("MissingStructClosingBrace", "struct Test {",
                {{ValuascriptErrorCode::ExpectedRightBraceAfterStructBody, 1, 14, 1, 15}},
                IsStructDef("Test", {}, {})
            );

            reg("MissingStructName", "struct { id: int }",
                {{ValuascriptErrorCode::ExpectedStructName, 1, 8, 1, 9}},
                IsStructDef("<error>", {}, {
                                {"id", {}, IsType("int")}
                            }
                )
            );

            reg("MissingStructComma", "struct Test { id: int name: string }",
                {{ValuascriptErrorCode::ExpectedCommaSeparatorInStruct, 1, 23, 1, 27}},
                IsStructDef("Test", {}, {
                                {"id", {}, IsType("int")},
                                {"name", {}, IsType("string")},
                            }
                )
            );

            reg("MissingStructLeftBrace", "struct Test id: int }",
                {{ValuascriptErrorCode::ExpectedBraceInStructDefinition, 1, 13, 1, 15}},
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
