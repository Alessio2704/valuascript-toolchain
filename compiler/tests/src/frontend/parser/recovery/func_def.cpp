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
                .errors = {{E::MissingTypeAnnotation, 1, 16, 1, 17}},
                .verifier = IsFunctionDef("test", {}, {}, {
                    IsNullType(),
                    IsType("int")
                })
            });

            reg({
                .name = "MissingArrowInFunction",
                .code = "func test(a: int) { return 1 }",
                .errors = {{E::MissingArrowInFunction, 1, 19, 1, 20}},
                .verifier = IsFunctionDef("test", {}, {{"a", {}, IsType("int")}}, {}, {IsReturn(IsNumber("1"))})
            });

            reg({
                .name = "MissingCommaInParamsRecoversAll",
                .code = "func test(a: int b: string) -> int {}",
                .errors = {{E::ExpectedCommaSeparatorInParameterList, 1, 18, 1, 19}},
                .verifier = IsFunctionDef("test", {}, {{"a", {}, IsType("int")}, {"b", {}, IsType("string")}}, {IsType("int")})
            });

            reg({
                .name = "MissingColonInParamsDiscardsParamAndRecovers",
                .code = "func test(a int, b: string) -> int {}",
                .errors = {{E::MissingColonAfterParameter, 1, 13, 1, 16}},
                .verifier = IsFunctionDef("test", {}, {{"<error>", {}, IsNullType()}, {"b", {}, IsType("string")}}, {IsType("int")})
            });

            reg({
                .name = "NoNameFuncEmptyAst",
                .code = "func (a: int) -> int {}",
                .errors = {{E::MissingFunctionName, 1, 6, 1, 7}},
                .verifier = IsFunctionDef("<error>", {}, {{"a", {}, IsType("int")}}, {IsType("int")})
            });

            reg({
                .name = "GarbageInParamsDiscardsAndRecovers",
                .code = "func test(a: int, *^, b: string) -> int {}",
                .errors = {{E::MissingParameterName, 1, 19, 1, 20}},
                .verifier = IsFunctionDef("test", {}, {{"a", {}, IsType("int")}, {"<error>", {}, IsNullType()}, {"b", {}, IsType("string")}}, {IsType("int")})
            });

            reg({
                .name = "MultipleReturnTypesMissingCommaRecovers",
                .code = "func test() -> int string {}",
                .errors = {{E::ExpectedCommaSeparatorInReturnTypeList, 1, 20, 1, 26}},
                .verifier = IsFunctionDef("test", {}, {}, {IsType("int"), IsType("string")})
            });

            reg({
                .name = "MissingColon",
                .code = "func test(a, b, c) -> int { return 1 }",
                .errors = {
                    {E::MissingColonAfterParameter, 1, 12, 1, 13},
                    {E::MissingColonAfterParameter, 1, 15, 1, 16},
                    {E::MissingColonAfterParameter, 1, 18, 1, 19}
                },
                .verifier = IsFunctionDef("test", {}, {{"a", {}, IsNullType()}, {"b", {}, IsNullType()}, {"c", {}, IsNullType()}}, {IsType("int")}, {IsReturn(IsNumber("1"))})
            });

            reg({
                .name = "MissingTypeAnnotationArguments1",
                .code = "func test(a: , b: , c: ) -> int { return 1 }",
                .errors = {
                    {E::MissingTypeAnnotation, 1, 14, 1, 15},
                    {E::MissingTypeAnnotation, 1, 19, 1, 20},
                    {E::MissingTypeAnnotation, 1, 24, 1, 25}
                },
                .verifier = IsFunctionDef("test", {}, {{"a", {}, IsNullType()}, {"b", {}, IsNullType()}, {"c", {}, IsNullType()}}, {IsType("int")}, {IsReturn(IsNumber("1"))})
            });

            reg({
                .name = "MissingTypeAnnotationArguments2",
                .code = "func test(a: int, b: , c: string d: decimal) -> int { return 1 }",
                .errors = {
                    {E::MissingTypeAnnotation, 1, 22, 1, 23},
                    {E::ExpectedCommaSeparatorInParameterList, 1, 34, 1, 35}
                },
                .verifier = IsFunctionDef("test", {}, {{"a", {}, IsType("int")}, {"b", {}, IsNullType()}, {"c", {}, IsType("string")}, {"d", {}, IsType("decimal")}}, {IsType("int")}, {IsReturn(IsNumber("1"))})
            });

            reg({
                .name = "MissingDefaultParameterValueSyncsToComma",
                .code = "func test(a: int =, b: string) -> int {}",
                .errors = {
                    {E::MissingDefaultParameterValue, 1, 19, 1, 20},
                    {E::NonDefaultParameterAfterDefault, 1, 21, 1, 22}
                },
                .verifier = IsFunctionDef("test", {}, {{"a", {}, IsType("int"), IsNull()}, {"b", {}, IsType("string")}}, {IsType("int")})
            });

            reg({
                .name = "MissingDefaultParameterValueSyncsToParen",
                .code = "func test(a: int =) -> int {}",
                .errors = {{E::MissingDefaultParameterValue, 1, 19, 1, 20}},
                .verifier = IsFunctionDef("test", {}, {{"a", {}, IsType("int"), IsNull()}}, {IsType("int")})
            });

            reg({
                .name = "NonDefaultParameterAfterDefaultReportsError",
                .code = "func test(a: int = 1, b: int) -> int {}",
                .errors = {{E::NonDefaultParameterAfterDefault, 1, 23, 1, 24}},
                .verifier = IsFunctionDef("test", {}, {{"a", {}, IsType("int"), IsNumber("1")}, {"b", {}, IsType("int")}}, {IsType("int")})
            });

            reg({
                .name = "InvalidExpressionInDefaultValueRecoversToComma",
                .code = "func test(a: int = *, b: string) -> int {}",
                .errors = {
                    {E::InvalidExpression, 1, 20, 1, 21},
                    {E::NonDefaultParameterAfterDefault, 1, 23, 1, 24}
                },
                .verifier = IsFunctionDef("test", {}, {{"a", {}, IsType("int"), IsNull()}, {"b", {}, IsType("string")}}, {IsType("int")})
            });

            reg({
                .name = "MultipleNonDefaultParametersAfterDefaultReportsMultipleErrors",
                .code = "func test(a: int = 1, b: int, c: int) -> int {}",
                .errors = {
                    {E::NonDefaultParameterAfterDefault, 1, 23, 1, 24},
                    {E::NonDefaultParameterAfterDefault, 1, 31, 1, 32}
                },
                .verifier = IsFunctionDef("test", {}, {{"a", {}, IsType("int"), IsNumber("1")}, {"b", {}, IsType("int")}, {"c", {}, IsType("int")}}, {IsType("int")})
            });

            reg({
                .name = "ErrorInParamsPreservesDocstring",
                .code = R"(func test(a: ) -> int { """docs""" })",
                .errors = {{E::MissingTypeAnnotation, 1, 14, 1, 15}},
                .verifier = IsFunctionDef("test", {}, {{"a", {}, IsNullType()}}, {IsType("int")}, {}, R"("""docs""")")
            });

            return true;
        }();
    }

    TEST_P(FuncDefErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectFunctionDefinitionErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
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
