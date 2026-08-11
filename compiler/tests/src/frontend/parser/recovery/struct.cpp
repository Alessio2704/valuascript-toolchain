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
                .errors = {PErr{.code = E::ExpectedRightBraceAfterStructBody, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}},
                .verifier = IsStructDef("Test", {}, {})
            });

            reg({
                .name = "MissingStructName",
                .code = "struct { id: int }",
                .errors = {PErr{.code = E::ExpectedStructName, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9}},
                .verifier = IsStructDef("<error>", {}, {
                    {.name="id", .modifiers={}, .type_v=IsType("int")}
                })
            });

            reg({
                .name = "MissingStructColumn",
                .code = "struct Test { id int }",
                .errors = {PErr{.code = E::ExpectedColonAfterStructFieldName, .line_start = 1, .column_start = 18, .line_end = 1, .column_end = 21}},
                .verifier = IsStructDef("Test", {}, {
                    {.name="<error>", .modifiers={}, .type_v=IsNullType()}
                })
            });

            reg({
                .name = "MissingStructFieldType",
                .code = "struct Test { id: }",
                .errors = {PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 19, .line_end = 1, .column_end = 20}},
                .verifier = IsStructDef("Test", {}, {{.name="id", .modifiers={}, .type_v=IsNullType()}})
            });

            reg({
                .name = "MissingStructComma",
                .code = "struct Test { id: int name: string }",
                .errors = {PErr{.code = E::ExpectedCommaSeparatorInStruct, .line_start = 1, .column_start = 23, .line_end = 1, .column_end = 27}},
                .verifier = IsStructDef("Test", {}, {
                    {.name="id", .modifiers={}, .type_v=IsType("int")},
                    {.name="name", .modifiers={}, .type_v=IsType("string")}
                })
            });

            reg({
                .name = "MissingStructLeftBrace",
                .code = "struct Test",
                .errors = {PErr{.code = E::ExpectedBraceInStructDefinition, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}},
                .verifier = IsStructDef("Test", {}, {})
            });

            reg({
                .name = "NoColonRecoversOtherFields",
                .code = "struct Test { host: string port: int speed int mode: string }",
                .errors = {
                    PErr{.code = E::ExpectedCommaSeparatorInStruct, .line_start = 1, .column_start = 28, .line_end = 1, .column_end = 32},
                    PErr{.code = E::ExpectedCommaSeparatorInStruct, .line_start = 1, .column_start = 38, .line_end = 1, .column_end = 43},
                    PErr{.code = E::ExpectedColonAfterStructFieldName, .line_start = 1, .column_start = 44, .line_end = 1, .column_end = 47}
                },
                .verifier = IsStructDef("Test", {}, {
                    {.name="host", .modifiers={}, .type_v=IsType("string")},
                    {.name="port", .modifiers={}, .type_v=IsType("int")},
                    {.name="<error>", .modifiers={}, .type_v=IsNullType()},
                    {.name="mode", .modifiers={}, .type_v=IsType("string")}
                })
            });

            reg({
                .name = "NoRightBraceStruct",
                .code = "struct Test { id: int ",
                .errors = {PErr{.code = E::ExpectedRightBraceAfterStructBody, .line_start = 1, .column_start = 21, .line_end = 1, .column_end = 22}},
                .verifier = IsStructDef("Test", {}, {
                    {.name="id", .modifiers={}, .type_v=IsType("int")}
                })
            });

            reg({
                .name = "NoFieldNameInStruct",
                .code = "struct Test { : int }",
                .errors = {PErr{.code = E::ExpectedStructFieldName, .line_start = 1, .column_start = 15, .line_end = 1, .column_end = 16}},
                .verifier = IsStructDef("Test", {}, {
                    {.name="<error>", .modifiers={}, .type_v=IsType("int")}
                })
            });

            reg({
                .name = "MissingCommasAndTrailingTypeInStruct",
                .code = "struct Test { host: string port: int speed: int mode: }",
                .errors = {
                    PErr{.code = E::ExpectedCommaSeparatorInStruct, .line_start = 1, .column_start = 28, .line_end = 1, .column_end = 32},
                    PErr{.code = E::ExpectedCommaSeparatorInStruct, .line_start = 1, .column_start = 38, .line_end = 1, .column_end = 43},
                    PErr{.code = E::ExpectedCommaSeparatorInStruct, .line_start = 1, .column_start = 49, .line_end = 1, .column_end = 53},
                    PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 55, .line_end = 1, .column_end = 56}
                },
                .verifier = IsStructDef("Test", {}, {
                    {.name="host", .modifiers={}, .type_v=IsType("string")},
                    {.name="port", .modifiers={}, .type_v=IsType("int")},
                    {.name="speed", .modifiers={}, .type_v=IsType("int")},
                    {.name="mode", .modifiers={}, .type_v=IsNullType()}
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
        TestNameGenerator{}
    );
}
