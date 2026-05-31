#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class TypeAliasErrorRegistryRunner : public ParserTestBase,
                                         public testing::WithParamInterface<ErrorRegistryEntry<AliasVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs,
                          const OneOf<AliasVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("MissingAliasName", "typealias = int",
                {
                    {E::ExpectedTypeAliasName, 1, 11, 1, 12}
                },
                IsTypeAlias("<error>", {}, IsType("int"))
            );

            reg("MissingTargetTypeAnnotation", "typealias MyType =",
                {
                    {E::MissingTypeAnnotation, 1, 19, 1, 20}
                },
                IsTypeAlias("MyType", {}, IsNullType())
            );

            reg("GarbageTargetTypeAnnotation", "typealias MyType = *",
                {
                    {E::MissingTypeAnnotation, 1, 20, 1, 21}
                },
                IsTypeAlias("MyType", {}, IsNullType())
            );

            reg("GarbageAtEndOfOtherwiseValidAlias", "typealias User = string ^^",
                {
                    {E::InvalidExpression, 1, 25, 1, 26}
                },
                IsTypeAlias("User", {}, IsType("string"))
            );


            return true;
        }();
    }

    TEST_P(TypeAliasErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectTypeAliasErrors(code, errors, verifier, skip_contexts);
    }

    INSTANTIATE_TEST_SUITE_P(
        Typealias,
        TypeAliasErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::aliases()),
        [](const testing::TestParamInfo<ErrorRegistryEntry<AliasVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
