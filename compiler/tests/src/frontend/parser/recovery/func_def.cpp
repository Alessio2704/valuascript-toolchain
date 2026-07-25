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

            reg("MissingArrowInFunction", "func test(a: int) { return 1 }",
                {{E::MissingArrowInFunction, 1, 19, 1, 20}},
                IsFunctionDef("test", {}, {{"a", {}, IsType("int")}}, {}, {IsReturn({IsNumber("1")})})
            );

            reg("MissingCommaInParamsRecoversAll", "func test(a: int b: string) -> int {}",
                {{E::ExpectedCommaSeparatorInParameterList, 1, 18, 1, 19}},
                IsFunctionDef("test", {}, {{"a", {}, IsType("int")}, {"b", {}, IsType("string")}}, {IsType("int")})
            );

            reg("MissingColonInParamsDiscardsParamAndRecovers", "func test(a int, b: string) -> int {}",
                {{E::MissingColonAfterParameter, 1, 13, 1, 16}},
                IsFunctionDef("test", {}, {{"<error>", {}, IsNullType()}, {"b", {}, IsType("string")}}, {IsType("int")})
            );

            reg("NoNameFuncEmptyAst", "func (a: int) -> int {}",
                {{E::MissingFunctionName, 1, 6, 1, 7}},
                IsFunctionDef("<error>", {}, {{"a", {}, IsType("int")}}, {IsType("int")})
            );

            reg("GarbageInParamsDiscardsAndRecovers", "func test(a: int, *^, b: string) -> int {}",
                {{E::MissingParameterName, 1, 19, 1, 20}},
                IsFunctionDef("test", {}, {{"a", {}, IsType("int")}, {"<error>", {}, IsNullType()}, {"b", {}, IsType("string")}}, {IsType("int")})
            );

            reg("MultipleReturnTypesMissingCommaRecovers", "func test() -> int string {}",
                {{E::ExpectedCommaSeparatorInReturnTypeList, 1, 20, 1, 26}},
                IsFunctionDef("test", {}, {}, {IsType("int"), IsType("string")})
            );

            reg("MissingColon", "func test(a, b, c) -> int { return 1 }",
                {
                    {E::MissingColonAfterParameter, 1, 12, 1, 13},
                    {E::MissingColonAfterParameter, 1, 15, 1, 16},
                    {E::MissingColonAfterParameter, 1, 18, 1, 19}
                },
                IsFunctionDef("test", {}, {{"a", {}, IsNullType()}, {"b", {}, IsNullType()}, {"c", {}, IsNullType()}}, {IsType("int")}, {IsReturn({IsNumber("1")})})
            );

            reg("MissingTypeAnnotationArguments1", "func test(a: , b: , c: ) -> int { return 1 }",
                {
                    {E::MissingTypeAnnotation, 1, 14, 1, 15},
                    {E::MissingTypeAnnotation, 1, 19, 1, 20},
                    {E::MissingTypeAnnotation, 1, 24, 1, 25}
                },
                IsFunctionDef("test", {}, {{"a", {}, IsNullType()}, {"b", {}, IsNullType()}, {"c", {}, IsNullType()}}, {IsType("int")}, {IsReturn({IsNumber("1")})})
            );

            reg("MissingTypeAnnotationArguments2", "func test(a: int, b: , c: string d: decimal) -> int { return 1 }",
                {
                    {E::MissingTypeAnnotation, 1, 22, 1, 23},
                    {E::ExpectedCommaSeparatorInParameterList, 1, 34, 1, 35}
                },
                IsFunctionDef("test", {}, {{"a", {}, IsType("int")}, {"b", {}, IsNullType()}, {"c", {}, IsType("string")}, {"d", {}, IsType("decimal")}}, {IsType("int")}, {IsReturn({IsNumber("1")})})
            );

            reg("MissingDefaultParameterValueSyncsToComma", "func test(a: int =, b: string) -> int {}",
                {
                    {E::MissingDefaultParameterValue, 1, 19, 1, 20},
                    {E::NonDefaultParameterAfterDefault, 1, 21, 1, 22}
                },
                IsFunctionDef("test", {}, {{"a", {}, IsType("int"), IsNull()}, {"b", {}, IsType("string")}}, {IsType("int")})
            );

            reg("MissingDefaultParameterValueSyncsToParen", "func test(a: int =) -> int {}",
                {{E::MissingDefaultParameterValue, 1, 19, 1, 20}},
                IsFunctionDef("test", {}, {{"a", {}, IsType("int"), IsNull()}}, {IsType("int")})
            );

            reg("NonDefaultParameterAfterDefaultReportsError", "func test(a: int = 1, b: int) -> int {}",
                {{E::NonDefaultParameterAfterDefault, 1, 23, 1, 24}},
                IsFunctionDef("test", {}, {{"a", {}, IsType("int"), IsNumber("1")}, {"b", {}, IsType("int")}}, {IsType("int")})
            );

            reg("InvalidExpressionInDefaultValueRecoversToComma", "func test(a: int = *, b: string) -> int {}",
                {
                    {E::InvalidExpression, 1, 20, 1, 21},
                    {E::NonDefaultParameterAfterDefault, 1, 23, 1, 24}
                },
                IsFunctionDef("test", {}, {{"a", {}, IsType("int"), IsNull()}, {"b", {}, IsType("string")}}, {IsType("int")})
            );

            reg("MultipleNonDefaultParametersAfterDefaultReportsMultipleErrors", "func test(a: int = 1, b: int, c: int) -> int {}",
                {
                    {E::NonDefaultParameterAfterDefault, 1, 23, 1, 24},
                    {E::NonDefaultParameterAfterDefault, 1, 31, 1, 32}
                },
                IsFunctionDef("test", {}, {{"a", {}, IsType("int"), IsNumber("1")}, {"b", {}, IsType("int")}, {"c", {}, IsType("int")}}, {IsType("int")})
            );

            reg("ErrorInParamsPreservesDocstring", R"(func test(a: ) -> int { """docs""" })",
                {{E::MissingTypeAnnotation, 1, 14, 1, 15}},
                IsFunctionDef("test", {}, {{"a", {}, IsNullType()}}, {IsType("int")}, {}, R"("""docs""")")
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
