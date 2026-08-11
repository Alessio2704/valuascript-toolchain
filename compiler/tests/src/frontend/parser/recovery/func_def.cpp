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
            auto reg = [](const RecoveryCase<FuncVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "MissingTypeAfterArrowDiscardsAndContinues",
                .code = "func test() -> , int {}",
                .errors = {
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 16, .line_end = 1,
                        .column_end = 17
                    }
                },
                .verifier = IsFunctionDef("test", {}, {}, {
                                              IsNullType(),
                                              IsType("int")
                                          })
            });

            reg({
                .name = "MissingArrowInFunction",
                .code = "func test(a: int) { return 1 }",
                .errors = {
                    PErr{
                        .code = E::MissingArrowInFunction, .line_start = 1, .column_start = 19, .line_end = 1,
                        .column_end = 20
                    }
                },
                .verifier = IsFunctionDef("test", {}, {{.name = "a", .modifiers = {}, .type_v = IsType("int")}}, {},
                                          {IsReturn(IsNumber("1"))})
            });

            reg({
                .name = "MissingRightBraceInFunctionBody",
                .code = "func test() -> int { return 1 ",
                .errors = {
                    PErr{
                        .code = E::ExpectedRightBraceAfterFunctionBody, .line_start = 1, .column_start = 29,
                        .line_end = 1, .column_end = 30
                    }
                },
                .verifier = IsFunctionDef("test", {}, {}, {IsType("int")}, {IsReturn(IsNumber("1"))}),
                .excluded_sentinels = {SentinelKind::Assignment, SentinelKind::Reassignment, SentinelKind::ExprStmt},
                .accepted_sentinels = SentinelKinds::all()
            });

            reg({
                .name = "DanglingModifierInFuncBody",
                .code = "func test() -> int { @ }",
                .errors = {
                    PErr{
                        .code = E::ExpectedModifierName, .line_start = 1, .column_start = 24,
                        .line_end = 1, .column_end = 25
                    },
                    PErr{
                        .code = E::ModifiersAttachedToInvalidDeclaration, .line_start = 1, .column_start = 22,
                        .line_end = 1, .column_end = 23
                    }
                },
                .verifier = IsFunctionDef("test", {}, {}, {IsType("int")}, {}),
                .excluded_sentinels = {SentinelKind::Assignment, SentinelKind::Reassignment, SentinelKind::ExprStmt},
                .accepted_sentinels = SentinelKinds::all()
            });

            reg({
                .name = "MissingCommaInParamsRecoversAll",
                .code = "func test(a: int b: string) -> int {}",
                .errors = {
                    PErr{
                        .code = E::ExpectedCommaSeparatorInParameterList, .line_start = 1, .column_start = 18,
                        .line_end = 1, .column_end = 19
                    }
                },
                .verifier = IsFunctionDef("test", {}, {
                                              {.name = "a", .modifiers = {}, .type_v = IsType("int")},
                                              {.name = "b", .modifiers = {}, .type_v = IsType("string")}
                                          }, {IsType("int")})
            });

            reg({
                .name = "MissingColonInParamsDiscardsParamAndRecovers",
                .code = "func test(a int, b: string) -> int {}",
                .errors = {
                    PErr{
                        .code = E::MissingColonAfterParameter, .line_start = 1, .column_start = 13, .line_end = 1,
                        .column_end = 16
                    }
                },
                .verifier = IsFunctionDef("test", {}, {
                                              {.name = "<error>", .modifiers = {}, .type_v = IsNullType()},
                                              {.name = "b", .modifiers = {}, .type_v = IsType("string")}
                                          }, {IsType("int")})
            });

            reg({
                .name = "NoNameFuncEmptyAst",
                .code = "func (a: int) -> int {}",
                .errors = {
                    PErr{
                        .code = E::MissingFunctionName, .line_start = 1, .column_start = 6, .line_end = 1,
                        .column_end = 7
                    }
                },
                .verifier = IsFunctionDef("<error>", {}, {{.name = "a", .modifiers = {}, .type_v = IsType("int")}},
                                          {IsType("int")})
            });

            reg({
                .name = "GarbageInParamsDiscardsAndRecovers",
                .code = "func test(a: int, *^, b: string) -> int {}",
                .errors = {
                    PErr{
                        .code = E::MissingParameterName, .line_start = 1, .column_start = 19, .line_end = 1,
                        .column_end = 20
                    }
                },
                .verifier = IsFunctionDef("test", {}, {
                                              {.name = "a", .modifiers = {}, .type_v = IsType("int")},
                                              {.name = "<error>", .modifiers = {}, .type_v = IsNullType()},
                                              {.name = "b", .modifiers = {}, .type_v = IsType("string")}
                                          }, {IsType("int")})
            });

            reg({
                .name = "MultipleReturnTypesMissingCommaRecovers",
                .code = "func test() -> int string {}",
                .errors = {
                    PErr{
                        .code = E::ExpectedCommaSeparatorInReturnTypeList, .line_start = 1, .column_start = 20,
                        .line_end = 1, .column_end = 26
                    }
                },
                .verifier = IsFunctionDef("test", {}, {}, {IsType("int"), IsType("string")})
            });

            reg({
                .name = "MissingColon",
                .code = "func test(a, b, c) -> int { return 1 }",
                .errors = {
                    PErr{
                        .code = E::MissingColonAfterParameter, .line_start = 1, .column_start = 12, .line_end = 1,
                        .column_end = 13
                    },
                    PErr{
                        .code = E::MissingColonAfterParameter, .line_start = 1, .column_start = 15, .line_end = 1,
                        .column_end = 16
                    },
                    PErr{
                        .code = E::MissingColonAfterParameter, .line_start = 1, .column_start = 18, .line_end = 1,
                        .column_end = 19
                    }
                },
                .verifier = IsFunctionDef("test", {}, {
                                              {.name = "a", .modifiers = {}, .type_v = IsNullType()},
                                              {.name = "b", .modifiers = {}, .type_v = IsNullType()},
                                              {.name = "c", .modifiers = {}, .type_v = IsNullType()}
                                          }, {IsType("int")}, {IsReturn(IsNumber("1"))})
            });

            reg({
                .name = "MissingTypeAnnotationArguments1",
                .code = "func test(a: , b: , c: ) -> int { return 1 }",
                .errors = {
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 14, .line_end = 1,
                        .column_end = 15
                    },
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 19, .line_end = 1,
                        .column_end = 20
                    },
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 24, .line_end = 1,
                        .column_end = 25
                    }
                },
                .verifier = IsFunctionDef("test", {}, {
                                              {.name = "a", .modifiers = {}, .type_v = IsNullType()},
                                              {.name = "b", .modifiers = {}, .type_v = IsNullType()},
                                              {.name = "c", .modifiers = {}, .type_v = IsNullType()}
                                          }, {IsType("int")}, {IsReturn(IsNumber("1"))})
            });

            reg({
                .name = "MissingTypeAnnotationArguments2",
                .code = "func test(a: int, b: , c: string d: decimal) -> int { return 1 }",
                .errors = {
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 22, .line_end = 1,
                        .column_end = 23
                    },
                    PErr{
                        .code = E::ExpectedCommaSeparatorInParameterList, .line_start = 1, .column_start = 34,
                        .line_end = 1, .column_end = 35
                    }
                },
                .verifier = IsFunctionDef("test", {}, {
                                              {.name = "a", .modifiers = {}, .type_v = IsType("int")},
                                              {.name = "b", .modifiers = {}, .type_v = IsNullType()},
                                              {.name = "c", .modifiers = {}, .type_v = IsType("string")},
                                              {.name = "d", .modifiers = {}, .type_v = IsType("decimal")}
                                          }, {IsType("int")}, {IsReturn(IsNumber("1"))})
            });

            reg({
                .name = "MissingDefaultParameterValueSyncsToComma",
                .code = "func test(a: int =, b: string) -> int {}",
                .errors = {
                    PErr{
                        .code = E::MissingDefaultParameterValue, .line_start = 1, .column_start = 19, .line_end = 1,
                        .column_end = 20
                    },
                    PErr{
                        .code = E::NonDefaultParameterAfterDefault, .line_start = 1, .column_start = 21, .line_end = 1,
                        .column_end = 22
                    }
                },
                .verifier = IsFunctionDef("test", {}, {
                                              {
                                                  .name = "a", .modifiers = {}, .type_v = IsType("int"),
                                                  .default_v = IsNull()
                                              },
                                              {.name = "b", .modifiers = {}, .type_v = IsType("string")}
                                          }, {IsType("int")})
            });

            reg({
                .name = "MissingDefaultParameterValueSyncsToParen",
                .code = "func test(a: int =) -> int {}",
                .errors = {
                    PErr{
                        .code = E::MissingDefaultParameterValue, .line_start = 1, .column_start = 19, .line_end = 1,
                        .column_end = 20
                    }
                },
                .verifier = IsFunctionDef("test", {}, {
                                              {
                                                  .name = "a", .modifiers = {}, .type_v = IsType("int"),
                                                  .default_v = IsNull()
                                              }
                                          }, {IsType("int")})
            });

            reg({
                .name = "NonDefaultParameterAfterDefaultReportsError",
                .code = "func test(a: int = 1, b: int) -> int {}",
                .errors = {
                    PErr{
                        .code = E::NonDefaultParameterAfterDefault, .line_start = 1, .column_start = 23, .line_end = 1,
                        .column_end = 24
                    }
                },
                .verifier = IsFunctionDef("test", {}, {
                                              {
                                                  .name = "a", .modifiers = {}, .type_v = IsType("int"),
                                                  .default_v = IsNumber("1")
                                              },
                                              {.name = "b", .modifiers = {}, .type_v = IsType("int")}
                                          }, {IsType("int")})
            });

            reg({
                .name = "InvalidExpressionInDefaultValueRecoversToComma",
                .code = "func test(a: int = *, b: string) -> int {}",
                .errors = {
                    PErr{
                        .code = E::InvalidExpression, .line_start = 1, .column_start = 20, .line_end = 1,
                        .column_end = 21
                    },
                    PErr{
                        .code = E::NonDefaultParameterAfterDefault, .line_start = 1, .column_start = 23, .line_end = 1,
                        .column_end = 24
                    }
                },
                .verifier = IsFunctionDef("test", {}, {
                                              {
                                                  .name = "a", .modifiers = {}, .type_v = IsType("int"),
                                                  .default_v = IsNull()
                                              },
                                              {.name = "b", .modifiers = {}, .type_v = IsType("string")}
                                          }, {IsType("int")})
            });

            reg({
                .name = "MultipleNonDefaultParametersAfterDefaultReportsMultipleErrors",
                .code = "func test(a: int = 1, b: int, c: int) -> int {}",
                .errors = {
                    PErr{
                        .code = E::NonDefaultParameterAfterDefault, .line_start = 1, .column_start = 23, .line_end = 1,
                        .column_end = 24
                    },
                    PErr{
                        .code = E::NonDefaultParameterAfterDefault, .line_start = 1, .column_start = 31, .line_end = 1,
                        .column_end = 32
                    }
                },
                .verifier = IsFunctionDef("test", {}, {
                                              {
                                                  .name = "a", .modifiers = {}, .type_v = IsType("int"),
                                                  .default_v = IsNumber("1")
                                              },
                                              {.name = "b", .modifiers = {}, .type_v = IsType("int")},
                                              {.name = "c", .modifiers = {}, .type_v = IsType("int")}
                                          }, {IsType("int")})
            });

            reg({
                .name = "ErrorInParamsPreservesDocstring",
                .code = R"(func test(a: ) -> int { """docs""" })",
                .errors = {
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 14, .line_end = 1,
                        .column_end = 15
                    }
                },
                .verifier = IsFunctionDef("test", {}, {{.name = "a", .modifiers = {}, .type_v = IsNullType()}},
                                          {IsType("int")}, {}, R"("""docs""")")
            });

            return true;
        }();
    }

    TEST_P(FuncDefErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels,
            accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectFunctionDefinitionErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels,
                                       accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        FunctionDefinition,
        FuncDefErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::functions()),
        TestNameGenerator{}
    );
}
