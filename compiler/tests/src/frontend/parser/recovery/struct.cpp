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
            auto reg = [](const RecoveryCase<StructVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "MissingStructClosingBrace",
                .code = "struct Test {",
                .errors = {{E::ExpectedRightBraceAfterStructBody, 1, 13, 1, 14}},
                .verifier = IsStructDef("Test", {}, {})
            });

            reg({
                .name = "MissingStructName",
                .code = "struct { id: int }",
                .errors = {{E::ExpectedStructName, 1, 8, 1, 9}},
                .verifier = IsStructDef("<error>", {}, {
                    {"id", {}, IsType("int")}
                })
            });

            reg({
                .name = "MissingStructColumn",
                .code = "struct Test { id int }",
                .errors = {{E::ExpectedColonAfterStructFieldName, 1, 18, 1, 21}},
                .verifier = IsStructDef("Test", {}, {
                    {"<error>", {}, IsNullType()}
                })
            });

            reg({
                .name = "MissingStructFieldType",
                .code = "struct Test { id: }",
                .errors = {{E::MissingTypeAnnotation, 1, 19, 1, 20}},
                .verifier = IsStructDef("Test", {}, {{"id", {}, IsNullType()}})
            });

            reg({
                .name = "MissingStructComma",
                .code = "struct Test { id: int name: string }",
                .errors = {{E::ExpectedCommaSeparatorInStruct, 1, 23, 1, 27}},
                .verifier = IsStructDef("Test", {}, {
                    {"id", {}, IsType("int")},
                    {"name", {}, IsType("string")}
                })
            });

            reg({
                .name = "MissingStructLeftBrace",
                .code = "struct Test id: int }",
                .errors = {{E::ExpectedBraceInStructDefinition, 1, 13, 1, 15}},
                .verifier = IsNull()
            });

            reg({
                .name = "NoColonRecoversOtherFields",
                .code = "struct Test { host: string port: int speed int mode: string }",
                .errors = {
                    {E::ExpectedCommaSeparatorInStruct, 1, 28, 1, 32},
                    {E::ExpectedCommaSeparatorInStruct, 1, 38, 1, 43},
                    {E::ExpectedColonAfterStructFieldName, 1, 44, 1, 47}
                },
                .verifier = IsStructDef("Test", {}, {
                    {"host", {}, IsType("string")},
                    {"port", {}, IsType("int")},
                    {"<error>", {}, IsNullType()}
                })
            });

            reg({
                .name = "NoRightBraceStruct",
                .code = "struct Test { id: int ",
                .errors = {{E::ExpectedRightBraceAfterStructBody, 1, 21, 1, 22}},
                .verifier = IsStructDef("Test", {}, {
                    {"id", {}, IsType("int")}
                })
            });

            reg({
                .name = "NoFieldNameInStruct",
                .code = "struct Test { : int }",
                .errors = {{E::ExpectedStructFieldName, 1, 15, 1, 16}},
                .verifier = IsStructDef("Test", {}, {
                    {"<error>", {}, IsType("int")}
                })
            });

            reg({
                .name = "MissingCommasAndTrailingTypeInStruct",
                .code = "struct Test { host: string port: int speed: int mode: }",
                .errors = {
                    {E::ExpectedCommaSeparatorInStruct, 1, 28, 1, 32},
                    {E::ExpectedCommaSeparatorInStruct, 1, 38, 1, 43},
                    {E::ExpectedCommaSeparatorInStruct, 1, 49, 1, 53},
                    {E::MissingTypeAnnotation, 1, 55, 1, 56}
                },
                .verifier = IsStructDef("Test", {}, {
                    {"host", {}, IsType("string")},
                    {"port", {}, IsType("int")},
                    {"speed", {}, IsType("int")},
                    {"mode", {}, IsNullType()}
                })
            });

            return true;
        }();
    }

    TEST_P(StructErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectStructDefinitionErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
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
