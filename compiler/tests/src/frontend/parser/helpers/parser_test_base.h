#pragma once

#include <gtest/gtest.h>

#include "recovery_sentinel.h"
#include "test_structures.h"
#include "core/compiler_context.h"
#include "expansion_calculator.h"

#include "utils/parametrised_test_name_helper.h"

namespace valuascript::compiler::test
{
    struct ConstructedRecoveryProgram
    {
        std::string full_code;
        ProgramSpec full_spec;
        std::string prefix_for_shifting;
    };

    using ExpansionCallback = std::function<void(ProcessingItem&&)>;

    class ParserTestBase : public testing::Test
    {
    protected:
        static std::string format_source_with_lines(const std::string& code);
        static std::shared_ptr<Program> run_parser(const std::string& code, CompilerContext& context);

        static void expand_to_top_level_stream(std::vector<ProcessingItem> items,
                                               const ExpansionCallback& callback,
                                               bool inject_sentinels = false);

        static std::vector<ProcessingItem> apply_context_augmentations(InjectableType type,
                                                                const std::string& snippet,
                                                                const UniversalVerifier& verifier,
                                                                const std::string& group_name);

        static ConstructedRecoveryProgram BuildRecoveryProgram(std::string inner_code,
                                                               ProgramSpec inner_spec,
                                                               const std::string& inner_prefix,
                                                               size_t seed);

        static void RunRecoveryScenario(ProcessingItem&& item, const std::vector<ParserExpectedError>& errors,
                                        size_t seed);

        static void ExpectValidParse(const std::string& code, const ProgramSpec& spec);
        static void ExpectParseErrors(const std::string& code, const std::vector<ParserExpectedError>& expected_errors,
                                      const std::optional<ProgramSpec>& spec = std::nullopt);
        static void ExpectParseErrorsWithRecovery(const std::string& code,
                                                  const std::vector<ParserExpectedError>& expected_errors,
                                                  ProgramSpec broken_part_spec);

        static void ExpectValidAssignment(const std::string& code_snippet, const AssignmentVerifier& verifier);
        static void ExpectValidReassignment(const std::string& code_snippet, const ReassignmentVerifier& verifier);
        static void ExpectValidExpressionStatement(const std::string& code_snippet, const ExprStmtVerifier& verifier);
        static void ExpectValidImport(const std::string& code_snippet, const ImportVerifier& verifier);
        static void ExpectValidDirective(const std::string& code_snippet, const DirectiveVerifier& verifier);
        static void ExpectValidFunctionDefinition(const std::string& code_snippet, const FuncVerifier& verifier);
        static void ExpectValidStructDefinition(const std::string& code_snippet, const StructVerifier& verifier);
        static void ExpectValidEnumDefinition(const std::string& code_snippet, const EnumVerifier& verifier);
        static void ExpectValidTypeAlias(const std::string& code_snippet, const AliasVerifier& verifier);
        static void ExpectValidExpression(const std::string& code_snippet, const ExprVerifier& verifier);
        static void ExpectValidTypeAnnotation(const std::string& code_snippet, const TypeVerifier& verifier);
        static void ExpectValidModifiers(const std::string& code_snippet, const ModifierVerifier& verifier);
        static void ExpectValidReturn(const std::string& code_snippet, const ReturnVerifier& verifier);

        static void ExpectAssignmentErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                           OneOf<AssignmentVerifier> v);

        static void ExpectReassignmentErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                             OneOf<ReassignmentVerifier> v);

        static void ExpectExpressionStatementErrors(const std::string& snippet,
                                                    const std::vector<ParserExpectedError>& errs,
                                                    OneOf<ExprStmtVerifier> v);

        static void ExpectImportErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                       OneOf<ImportVerifier> v);

        static void ExpectDirectiveErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                          OneOf<DirectiveVerifier> v);

        static void ExpectFunctionDefinitionErrors(const std::string& snippet,
                                                   const std::vector<ParserExpectedError>& errs,
                                                   OneOf<FuncVerifier> v);

        static void ExpectStructDefinitionErrors(const std::string& snippet,
                                                 const std::vector<ParserExpectedError>& errs,
                                                 OneOf<StructVerifier> v);

        static void ExpectEnumDefinitionErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                               OneOf<EnumVerifier> v);

        static void ExpectTypeAliasErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                          OneOf<AliasVerifier> v);

        static void ExpectExpressionErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                           OneOf<ExprVerifier> v);

        static void ExpectTypeAnnotationErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                               OneOf<TypeVerifier> v);

        static void ExpectModifierErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                         OneOf<ModifierVerifier> v);

        static void ExpectReturnErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                       OneOf<ReturnVerifier> v);

        static void ExpectValidUnified(InjectableType type, std::vector<ProcessingItem> items,
                                       const std::string& group_name);

        template <typename Verifier>
        static void ExpectValidUnified(InjectableType type, const std::string& snippet,
                                       const Verifier& verifier, const std::string& group_name)
        {
            auto items = apply_context_augmentations(type, snippet, verifier, group_name);
            ExpectValidUnified(type, std::move(items), group_name);
        }

        static void ExpectParseErrorsUnified(InjectableType type, std::vector<ProcessingItem> items,
                                             const std::vector<ParserExpectedError>& errors,
                                             const std::string& group_name);

        template <typename Verifier>
        static void ExpectParseErrorsUnified(InjectableType type, const std::string& snippet,
                                             const std::vector<ParserExpectedError>& errors,
                                             const Verifier& verifier, const std::string& group_name)
        {
            auto items = apply_context_augmentations(type, snippet, verifier, group_name);
            ExpectParseErrorsUnified(type, std::move(items), errors, group_name);
        }
    };
}
