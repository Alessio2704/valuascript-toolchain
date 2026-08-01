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
            auto reg = [](const RecoveryCase<AliasVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "MissingAliasName",
                .code = "typealias = int",
                .errors = {
                    {E::ExpectedTypeAliasName, 1, 11, 1, 12}
                },
                .verifier = IsTypeAlias("<error>", {}, IsType("int"))
            });

            reg({
                .name = "MissingTargetTypeAnnotation",
                .code = "typealias MyType =",
                .errors = {
                    {E::MissingTypeAnnotation, 1, 19, 1, 20}
                },
                .verifier = IsTypeAlias("MyType", {}, IsNullType())
            });

            reg({
                .name = "GarbageTargetTypeAnnotation",
                .code = "typealias MyType = *",
                .errors = {
                    {E::MissingTypeAnnotation, 1, 20, 1, 21}
                },
                .verifier = IsTypeAlias("MyType", {}, IsNullType())
            });

            reg({
                .name = "GarbageAtEndOfOtherwiseValidAlias",
                .code = "typealias User = string ^^",
                .errors = {
                    {E::InvalidExpression, 1, 25, 1, 26}
                },
                .verifier = IsTypeAlias("User", {}, IsType("string"))
            });

            return true;
        }();
    }

    TEST_P(TypeAliasErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectTypeAliasErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
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
