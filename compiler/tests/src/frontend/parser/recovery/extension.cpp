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
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs, const OneOf<ExtVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
            };

            reg("MissingTypeName", "extension {}",
                {{E::MissingTypeAnnotation, 1, 11, 1, 12}},
                IsExtensionDef({}, IsNullType(), {})
            );

            reg("MissingTypeNameWithTrailingCharacters", "extension 123 {}",
                {{E::MissingTypeAnnotation, 1, 11, 1, 14}},
                IsExtensionDef({}, IsNullType(), {})
            );

            reg("MissingBrace", "extension Target",
                {{E::ExpectedLeftBraceBeforeExtensionBody, 1, 17, 1, 18}},
                IsExtensionDef({}, IsType("Target"), {})
            );

            reg("ModifierMissingTarget", "@* extension Target {}",
                {{E::ExpectedModifierName, 1, 2, 1, 3}},
                IsExtensionDef({}, IsType("Target"), {})
            );

            reg("ForbiddenImport", "extension Target { import \"abc\" }",
                {{E::ImportNotAllowedInExtension, 1, 20, 1, 26}},
                IsExtensionDef({}, IsType("Target"), {})
            );

            reg("ForbiddenDirective1", "extension Target { #no_value }",
                {{E::DirectiveNotAllowedInExtension, 1, 20, 1, 29}},
                IsExtensionDef({}, IsType("Target"), {})
            );

            reg("ForbiddenDirective2", "extension Target { #value = 10 }",
                {{E::DirectiveNotAllowedInExtension, 1, 20, 1, 31}},
                IsExtensionDef({}, IsType("Target"), {})
            );

            reg("ForbiddenDirective3", "extension Target { #value 10 }",
                {{E::DirectiveNotAllowedInExtension, 1, 20, 1, 29}},
                IsExtensionDef({}, IsType("Target"), {})
            );

            return true;
        }();
    }

    TEST_P(ExtensionErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts] = GetParam();
        SCOPED_TRACE("Running Recovery Test Case: " + name);
        ExpectExtensionDefinitionErrors(code, errors, verifier, skip_contexts);
    }

    INSTANTIATE_TEST_SUITE_P(
        ExtensionError,
        ExtensionErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::extensions()),
        [](const testing::TestParamInfo<ErrorRegistryEntry<ExtVerifier>>& test_info) {
        return test_info.param.test_name;
        }
    );
}
