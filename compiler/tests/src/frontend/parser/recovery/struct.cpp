#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class StructErrorRegistryRunner : public ParserTestBase,
                                      public testing::WithParamInterface<ErrorRegistryEntry<StructVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<StructVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            // reg("OnlyStruct", "struct",
            //     {{E::ExpectedStructName, 1, 8, 1, 9}},
            //     IsStructDef("<error>", {}, {})
            // );

            // reg("OnlyStructName", "struct Test",
            //     {{E::ExpectedBraceInStructDefinition, 1, 12, 1, 13}},
            //     IsStructDef("Test", {}, {})
            // );

            reg("MissingStructClosingBrace", "struct Test {",
                {{E::ExpectedRightBraceAfterStructBody, 1, 14, 1, 15}},
                IsStructDef("Test", {}, {})
            );

            reg("MissingStructName", "struct { id: int }",
                {{E::ExpectedStructName, 1, 8, 1, 9}},
                IsStructDef("<error>", {}, {
                                {"id", {}, IsType("int")}
                            }
                )
            );

            reg("MissingStructColumn", "struct Test { id int }",
                {{E::ExpectedColonAfterStructFieldName, 1, 18, 1, 21}},
                IsStructDef("Test", {}, {
                                {"<error>", {}, IsNullType()}
                            }
                )
            );

            reg("MissingStructFieldType", "struct Test { id: }",
                {{E::MissingTypeAnnotation, 1, 19, 1, 20}},
                IsStructDef("Test", {}, {{"id", {}, IsNullType()}})
            );

            reg("MissingStructComma", "struct Test { id: int name: string }",
                {{E::ExpectedCommaSeparatorInStruct, 1, 23, 1, 27}},
                IsStructDef("Test", {}, {
                                {"id", {}, IsType("int")},
                                {"name", {}, IsType("string")},
                            }
                )
            );

            reg("MissingStructLeftBrace", "struct Test id: int }",
                {{E::ExpectedBraceInStructDefinition, 1, 13, 1, 15}},
                IsNull()
            );

            reg("NoColonRecoversOtherFields", "struct Test { host: string port: int speed int mode: string }",
                {
                    {E::ExpectedCommaSeparatorInStruct, 1, 28, 1, 32},
                    {E::ExpectedCommaSeparatorInStruct, 1, 38, 1, 43},
                    {E::ExpectedColonAfterStructFieldName, 1, 44, 1, 47}
                },
                IsStructDef("Test", {}, {
                                {"host", {}, IsType("string")},
                                {"port", {}, IsType("int")},
                                {"<error>", {}, IsNullType()},
                            }
                )
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
