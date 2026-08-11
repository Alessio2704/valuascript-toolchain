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
                    PErr{
                        .code = E::ExpectedTypeAliasName, .line_start = 1, .column_start = 11, .line_end = 1,
                        .column_end = 12
                    }
                },
                .verifier = IsTypeAlias("<error>", {}, IsType("int"))
            });

            reg({
                .name = "MissingTargetTypeAnnotation",
                .code = "typealias MyType =",
                .errors = {
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 19, .line_end = 1,
                        .column_end = 20
                    }
                },
                .verifier = IsTypeAlias("MyType", {}, IsNullType())
            });

            reg({
                .name = "GarbageTargetTypeAnnotation",
                .code = "typealias MyType = *",
                .errors = {
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 20, .line_end = 1,
                        .column_end = 21
                    }
                },
                .verifier = IsTypeAlias("MyType", {}, IsNullType())
            });

            reg({
                .name = "TypeAliasMissingEquals",
                .code = "typealias MyType int",
                .errors = {
                    PErr{
                        .code = E::ExpectedAssignAfterTypeAliasName, .line_start = 1, .column_start = 17, .line_end = 1,
                        .column_end = 18
                    }
                },
                .verifier = IsTypeAlias("MyType", {}, IsType("int"))
            });

            reg({
                .name = "TypeAliasMissingEqualsWithComplexType",
                .code = "typealias MyType vector<int>",
                .errors = {
                    PErr{
                        .code = E::ExpectedAssignAfterTypeAliasName, .line_start = 1, .column_start = 17, .line_end = 1,
                        .column_end = 18
                    }
                },
                .verifier = IsTypeAlias("MyType", {}, IsType("vector", IsType("int")))
            });

            return true;
        }();
    }

    TEST_P(TypeAliasErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels,
            accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectTypeAliasErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels,
                              accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        Typealias,
        TypeAliasErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::aliases()),
        TestNameGenerator{}
    );
}
