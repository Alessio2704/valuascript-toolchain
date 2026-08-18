#include "parser_test_base.h"

namespace valuascript::compiler::test
{
    void ParserTestBase::ExpectValidAssignment(const std::string& snippet, const AssignmentVerifier& v,
                                               const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::StrongStatement, snippet, StmtVerifier(v), "Assignment", skip_contexts);
    }

    void ParserTestBase::ExpectValidReassignment(const std::string& snippet, const ReassignmentVerifier& v,
                                                 const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::StrongStatement, snippet, StmtVerifier(v), "Reassignment", skip_contexts);
    }

    void ParserTestBase::ExpectValidExpressionStatement(const std::string& snippet, const ExprStmtVerifier& v,
                                                        const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::StrongStatement, snippet, StmtVerifier(v), "Expression Statement",
                           skip_contexts);
    }

    void ParserTestBase::ExpectValidImport(const std::string& snippet, const ImportVerifier& v,
                                           const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::Import, snippet, v, "Import", skip_contexts);
    }

    void ParserTestBase::ExpectValidDirective(const std::string& snippet, const DirectiveVerifier& v,
                                              const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::Directive, snippet, v, "Directive", skip_contexts);
    }

    void ParserTestBase::ExpectValidFunctionDefinition(const std::string& snippet, const FuncVerifier& v,
                                                       const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::Function, snippet, v, "Function", skip_contexts);
    }

    void ParserTestBase::ExpectValidExtensionDefinition(const std::string& snippet, const ExtVerifier& v,
                                                        const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::Extension, snippet, v, "Extension", skip_contexts);
    }

    void ParserTestBase::ExpectValidStructDefinition(const std::string& snippet, const StructVerifier& v,
                                                     const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::Struct, snippet, v, "Struct", skip_contexts);
    }

    void ParserTestBase::ExpectValidEnumDefinition(const std::string& snippet, const EnumVerifier& v,
                                                   const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::Enum, snippet, v, "Enum", skip_contexts);
    }

    void ParserTestBase::ExpectValidTypeAlias(const std::string& snippet, const AliasVerifier& v,
                                              const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::TypeAlias, snippet, v, "Type Alias", skip_contexts);
    }

    void ParserTestBase::ExpectValidExpression(const std::string& snippet, const ExprVerifier& v,
                                               const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::Expression, snippet, v, "Expression", skip_contexts);
    }

    void ParserTestBase::ExpectValidTypeAnnotation(const std::string& snippet, const TypeVerifier& v,
                                                   const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::TypeAnnotation, snippet, v, "Type Annotation", skip_contexts);
    }

    void ParserTestBase::ExpectValidModifiers(const std::string& snippet, const ModifierVerifier& v,
                                              const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::Modifier, snippet, v, "Modifier", skip_contexts);
    }

    void ParserTestBase::ExpectValidReturn(const std::string& snippet, const ReturnVerifier& v,
                                           const std::vector<std::string_view>& skip_contexts)
    {
        ExpectValidUnified(InjectableType::WeakStatement, snippet, v, "Return", skip_contexts);
    }

    void ParserTestBase::ExpectAssignmentErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                const OneOf<AssignmentVerifier>& v,
                                                const std::vector<std::string_view>& skip_contexts,
                                                const std::vector<ContextOverride<AssignmentVerifier>>& context_overrides,
                                                const std::vector<SentinelKind>& excluded_sentinels,
                                                const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::StrongStatement, s, e, v.value, "Assignment", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::ExpectReassignmentErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                  const OneOf<ReassignmentVerifier>& v,
                                                  const std::vector<std::string_view>& skip_contexts,
                                                  const std::vector<ContextOverride<ReassignmentVerifier>>& context_overrides,
                                                  const std::vector<SentinelKind>& excluded_sentinels,
                                                  const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::StrongStatement, s, e, v.value, "Reassignment", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::ExpectExpressionStatementErrors(const std::string& s,
                                                         const std::vector<ParserExpectedError>& e,
                                                         const OneOf<ExprStmtVerifier>& v,
                                                         const std::vector<std::string_view>& skip_contexts,
                                                         const std::vector<ContextOverride<ExprStmtVerifier>>& context_overrides,
                                                         const std::vector<SentinelKind>& excluded_sentinels,
                                                         const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::StrongStatement, s, e, v.value, "Expression Statement", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::ExpectImportErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                            const OneOf<ImportVerifier>& v,
                                            const std::vector<std::string_view>& skip_contexts,
                                            const std::vector<ContextOverride<ImportVerifier>>& context_overrides,
                                            const std::vector<SentinelKind>& excluded_sentinels,
                                            const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::Import, s, e, v.value, "Import", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::ExpectDirectiveErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                               const OneOf<DirectiveVerifier>& v,
                                               const std::vector<std::string_view>& skip_contexts,
                                               const std::vector<ContextOverride<DirectiveVerifier>>& context_overrides,
                                               const std::vector<SentinelKind>& excluded_sentinels,
                                               const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::Directive, s, e, v.value, "Directive", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::ExpectFunctionDefinitionErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                        const OneOf<FuncVerifier>& v,
                                                        const std::vector<std::string_view>& skip_contexts,
                                                        const std::vector<ContextOverride<FuncVerifier>>& context_overrides,
                                                        const std::vector<SentinelKind>& excluded_sentinels,
                                                        const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::Function, s, e, v.value, "Function Definition", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::ExpectExtensionDefinitionErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                         const OneOf<ExtVerifier>& v,
                                                         const std::vector<std::string_view>& skip_contexts,
                                                         const std::vector<ContextOverride<ExtVerifier>>& context_overrides,
                                                         const std::vector<SentinelKind>& excluded_sentinels,
                                                         const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::Extension, s, e, v.value, "Extension Definition", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::ExpectStructDefinitionErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                      const OneOf<StructVerifier>& v,
                                                      const std::vector<std::string_view>& skip_contexts,
                                                      const std::vector<ContextOverride<StructVerifier>>& context_overrides,
                                                      const std::vector<SentinelKind>& excluded_sentinels,
                                                      const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::Struct, s, e, v.value, "Struct Definition", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::ExpectEnumDefinitionErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                    const OneOf<EnumVerifier>& v,
                                                    const std::vector<std::string_view>& skip_contexts,
                                                    const std::vector<ContextOverride<EnumVerifier>>& context_overrides,
                                                    const std::vector<SentinelKind>& excluded_sentinels,
                                                    const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::Enum, s, e, v.value, "Enum Definition", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::ExpectTypeAliasErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                               const OneOf<AliasVerifier>& v,
                                               const std::vector<std::string_view>& skip_contexts,
                                               const std::vector<ContextOverride<AliasVerifier>>& context_overrides,
                                               const std::vector<SentinelKind>& excluded_sentinels,
                                               const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::TypeAlias, s, e, v.value, "Type Alias", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::ExpectExpressionErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                const OneOf<ExprVerifier>& v,
                                                const std::vector<std::string_view>& skip_contexts,
                                                const std::vector<ContextOverride<ExprVerifier>>& context_overrides,
                                                const std::vector<SentinelKind>& excluded_sentinels,
                                                const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::Expression, s, e, v.value, "Expression", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::ExpectTypeAnnotationErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                    const OneOf<TypeVerifier>& v,
                                                    const std::vector<std::string_view>& skip_contexts,
                                                    const std::vector<ContextOverride<TypeVerifier>>& context_overrides,
                                                    const std::vector<SentinelKind>& excluded_sentinels,
                                                    const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::TypeAnnotation, s, e, v.value, "Type Annotation", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::ExpectModifierErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                              const OneOf<ModifierVerifier>& v,
                                              const std::vector<std::string_view>& skip_contexts,
                                              const std::vector<ContextOverride<ModifierVerifier>>& context_overrides,
                                              const std::vector<SentinelKind>& excluded_sentinels,
                                              const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::Modifier, s, e, v.value, "Modifier", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::ExpectReturnErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                            const OneOf<ReturnVerifier>& v,
                                            const std::vector<std::string_view>& skip_contexts,
                                            const std::vector<ContextOverride<ReturnVerifier>>& context_overrides,
                                            const std::vector<SentinelKind>& excluded_sentinels,
                                            const std::vector<SentinelKind>& accepted_sentinels)
    {
        ExpectParseErrorsUnified(InjectableType::WeakStatement, s, e, v.value, "Return", skip_contexts, to_any_overrides(context_overrides), excluded_sentinels, accepted_sentinels);
    }
}
