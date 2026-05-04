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
                                                OneOf<AssignmentVerifier> v)
    {
        ExpectParseErrorsUnified(InjectableType::StrongStatement, s, e, v.value, "Assignment");
    }

    void ParserTestBase::ExpectReassignmentErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                  OneOf<ReassignmentVerifier> v)
    {
        ExpectParseErrorsUnified(InjectableType::StrongStatement, s, e, v.value, "Reassignment");
    }

    void ParserTestBase::ExpectExpressionStatementErrors(const std::string& s,
                                                         const std::vector<ParserExpectedError>& e,
                                                         OneOf<ExprStmtVerifier> v)
    {
        ExpectParseErrorsUnified(InjectableType::StrongStatement, s, e, v.value, "Expression Statement");
    }

    void ParserTestBase::ExpectImportErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                            OneOf<ImportVerifier> v)
    {
        ExpectParseErrorsUnified(InjectableType::Import, s, e, v.value, "Import");
    }

    void ParserTestBase::ExpectDirectiveErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                               OneOf<DirectiveVerifier> v)
    {
        ExpectParseErrorsUnified(InjectableType::Directive, s, e, v.value, "Directive");
    }

    void ParserTestBase::ExpectFunctionDefinitionErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                        OneOf<FuncVerifier> v)
    {
        ExpectParseErrorsUnified(InjectableType::Function, s, e, v.value, "Function Definition");
    }

    void ParserTestBase::ExpectStructDefinitionErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                      OneOf<StructVerifier> v)
    {
        ExpectParseErrorsUnified(InjectableType::Struct, s, e, v.value, "Struct Definition");
    }

    void ParserTestBase::ExpectEnumDefinitionErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                    OneOf<EnumVerifier> v)
    {
        ExpectParseErrorsUnified(InjectableType::Enum, s, e, v.value, "Enum Definition");
    }

    void ParserTestBase::ExpectTypeAliasErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                               OneOf<AliasVerifier> v)
    {
        ExpectParseErrorsUnified(InjectableType::TypeAlias, s, e, v.value, "Type Alias");
    }

    void ParserTestBase::ExpectExpressionErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                OneOf<ExprVerifier> v)
    {
        ExpectParseErrorsUnified(InjectableType::Expression, s, e, v.value, "Expression");
    }

    void ParserTestBase::ExpectTypeAnnotationErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                                    OneOf<TypeVerifier> v)
    {
        ExpectParseErrorsUnified(InjectableType::TypeAnnotation, s, e, v.value, "Type Annotation");
    }

    void ParserTestBase::ExpectModifierErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                              OneOf<ModifierVerifier> v)
    {
        ExpectParseErrorsUnified(InjectableType::Modifier, s, e, v.value, "Modifier");
    }

    void ParserTestBase::ExpectReturnErrors(const std::string& s, const std::vector<ParserExpectedError>& e,
                                            OneOf<ReturnVerifier> v)
    {
        ExpectParseErrorsUnified(InjectableType::WeakStatement, s, e, v.value, "Return");
    }
}
