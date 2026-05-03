#include "parser_test_base.h"

namespace valuascript::compiler::test
{
    void ParserTestBase::ExpectValidAssignment(const std::string& snippet, const AssignmentVerifier& v)
    {
        ExpectValidUnified(InjectableType::StrongStatement, snippet, StmtVerifier(v), "Assignment");
    }

    void ParserTestBase::ExpectValidReassignment(const std::string& snippet, const ReassignmentVerifier& v)
    {
        ExpectValidUnified(InjectableType::StrongStatement, snippet, StmtVerifier(v), "Reassignment");
    }

    void ParserTestBase::ExpectValidExpressionStatement(const std::string& snippet, const ExprStmtVerifier& v)
    {
        ExpectValidUnified(InjectableType::StrongStatement, snippet, StmtVerifier(v), "Expression Statement");
    }

    void ParserTestBase::ExpectValidImport(const std::string& snippet, const ImportVerifier& v)
    {
        ExpectValidUnified(InjectableType::Import, snippet, v, "Import");
    }

    void ParserTestBase::ExpectValidDirective(const std::string& snippet, const DirectiveVerifier& v)
    {
        ExpectValidUnified(InjectableType::Directive, snippet, v, "Directive");
    }

    void ParserTestBase::ExpectValidFunctionDefinition(const std::string& snippet, const FuncVerifier& v)
    {
        ExpectValidUnified(InjectableType::Function, snippet, v, "Function");
    }

    void ParserTestBase::ExpectValidStructDefinition(const std::string& snippet, const StructVerifier& v)
    {
        ExpectValidUnified(InjectableType::Struct, snippet, v, "Struct");
    }

    void ParserTestBase::ExpectValidEnumDefinition(const std::string& snippet, const EnumVerifier& v)
    {
        ExpectValidUnified(InjectableType::Enum, snippet, v, "Enum");
    }

    void ParserTestBase::ExpectValidTypeAlias(const std::string& snippet, const AliasVerifier& v)
    {
        ExpectValidUnified(InjectableType::TypeAlias, snippet, v, "Type Alias");
    }

    void ParserTestBase::ExpectValidExpression(const std::string& snippet, const ExprVerifier& v)
    {
        ExpectValidUnified(InjectableType::Expression, snippet, v, "Expression");
    }

    void ParserTestBase::ExpectValidTypeAnnotation(const std::string& snippet, const TypeVerifier& v)
    {
        ExpectValidUnified(InjectableType::TypeAnnotation, snippet, v, "Type Annotation");
    }

    void ParserTestBase::ExpectValidModifiers(const std::string& snippet, const ModifierVerifier& v)
    {
        ExpectValidUnified(InjectableType::Modifier, snippet, v, "Modifier");
    }

    void ParserTestBase::ExpectValidReturn(const std::string& snippet, const ReturnVerifier& v)
    {
        ExpectValidUnified(InjectableType::WeakStatement, snippet, v, "Return");
    }

    void ParserTestBase::ExpectAssignmentErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                const AssignmentVerifier& v)
    {
        ExpectParseErrorsUnified(InjectableType::StrongStatement, s, e, StmtVerifier(v), "Assignment");
    }

    void ParserTestBase::ExpectReassignmentErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                  const ReassignmentVerifier& v)
    {
        ExpectParseErrorsUnified(InjectableType::StrongStatement, s, e, StmtVerifier(v), "Reassignment");
    }

    void ParserTestBase::ExpectExpressionStatementErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                         const ExprStmtVerifier& v)
    {
        ExpectParseErrorsUnified(InjectableType::StrongStatement, s, e, StmtVerifier(v), "Expression Statement");
    }

    void ParserTestBase::ExpectImportErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                            const ImportVerifier& v)
    {
        ExpectParseErrorsUnified(InjectableType::Import, s, e, v, "Import");
    }

    void ParserTestBase::ExpectDirectiveErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                               const DirectiveVerifier& v)
    {
        ExpectParseErrorsUnified(InjectableType::Directive, s, e, v, "Directive");
    }

    void ParserTestBase::ExpectFunctionDefinitionErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                        const FuncVerifier& v)
    {
        ExpectParseErrorsUnified(InjectableType::Function, s, e, v, "Function Definition");
    }

    void ParserTestBase::ExpectStructDefinitionErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                      const StructVerifier& v)
    {
        ExpectParseErrorsUnified(InjectableType::Struct, s, e, v, "Struct Definition");
    }

    void ParserTestBase::ExpectEnumDefinitionErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                    const EnumVerifier& v)
    {
        ExpectParseErrorsUnified(InjectableType::Enum, s, e, v, "Enum Definition");
    }

    void ParserTestBase::ExpectTypeAliasErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                               const AliasVerifier& v)
    {
        ExpectParseErrorsUnified(InjectableType::TypeAlias, s, e, v, "Type Alias");
    }

    void ParserTestBase::ExpectExpressionErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                const ExprVerifier& v)
    {
        ExpectParseErrorsUnified(InjectableType::Expression, s, e, v, "Expression");
    }

    void ParserTestBase::ExpectTypeAnnotationErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                    const TypeVerifier& v)
    {
        ExpectParseErrorsUnified(InjectableType::TypeAnnotation, s, e, v, "Type Annotation");
    }

    void ParserTestBase::ExpectModifierErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                              const ModifierVerifier& v)
    {
        ExpectParseErrorsUnified(InjectableType::Modifier, s, e, v, "Modifier");
    }

    void ParserTestBase::ExpectReturnErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                            const ReturnVerifier& v)
    {
        ExpectParseErrorsUnified(InjectableType::WeakStatement, s, e, v, "Return");
    }
}
