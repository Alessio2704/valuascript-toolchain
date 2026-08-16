#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ExtensionErrorRegistryRunner : public ParserTestBase,
                                         public testing::WithParamInterface<ErrorRegistryEntry<ExtVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<ExtVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "MissingTypeName",
                .code = "extension {}",
                .errors = {PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}},
                .verifier = IsExtensionDef({}, IsNullType(), {})
            });

            reg({
                .name = "MissingTypeNameWithTrailingCharacters",
                .code = "extension 123 {}",
                .errors = {PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 14}},
                .verifier = IsExtensionDef({}, IsNullType(), {})
            });

            reg({
                .name = "MissingBrace",
                .code = "extension Target",
                .errors = {PErr{.code = E::ExpectedLeftBraceBeforeExtensionBody, .line_start = 1, .column_start = 17, .line_end = 1, .column_end = 18}},
                .verifier = IsExtensionDef({}, IsType("Target"), {})
            });

            reg({
                .name = "ModifierMissingTarget",
                .code = "@* extension Target {}",
                .errors = {PErr{.code = E::ExpectedModifierName, .line_start = 1, .column_start = 2, .line_end = 1, .column_end = 3}},
                .verifier = IsExtensionDef({}, IsType("Target"), {})
            });

            return true;
        }();
    }

    TEST_P(ExtensionErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Recovery Test Case: " + name);
        ExpectExtensionDefinitionErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        ExtensionError,
        ExtensionErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::extensions()),
        TestNameGenerator{}
    );
}
