#pragma once

#include <gtest/gtest.h>

#include "recovery_sentinel.h"
#include "test_structures.h"
#include "core/compiler_context.h"

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

        static void expand_to_top_level_stream(ProcessingItem&& item,
                                               const ExpansionCallback& callback,
                                               bool inject_sentinels = false);

        static ConstructedRecoveryProgram BuildRecoveryProgram(std::string inner_code,
                                                               ProgramSpec inner_spec,
                                                               const std::string& inner_prefix,
                                                               size_t seed);

        static void RunRecoveryScenario(ProcessingItem&& item, const std::vector<ExpectedError>& errors, size_t seed);

        static void ExpectValidParse(const std::string& code, const ProgramSpec& spec);
        static void ExpectParseErrors(const std::string& code, const std::vector<ExpectedError>& expected_errors,
                                      const std::optional<ProgramSpec>& spec = std::nullopt);
        static void ExpectParseErrorsWithRecovery(const std::string& code,
                                                  const std::vector<ExpectedError>& expected_errors,
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

        static void ExpectAssignmentErrors(const std::string& snippet, const std::vector<ExpectedError>& errs,
                                           const AssignmentVerifier& v);
        static void ExpectReassignmentErrors(const std::string& snippet, const std::vector<ExpectedError>& errs,
                                             const ReassignmentVerifier& v);
        static void ExpectExpressionStatementErrors(const std::string& snippet, const std::vector<ExpectedError>& errs,
                                                    const ExprStmtVerifier& v);
        static void ExpectImportErrors(const std::string& snippet, const std::vector<ExpectedError>& errs,
                                       const ImportVerifier& v);
        static void ExpectDirectiveErrors(const std::string& snippet, const std::vector<ExpectedError>& errs,
                                          const DirectiveVerifier& v);
        static void ExpectFunctionDefinitionErrors(const std::string& snippet, const std::vector<ExpectedError>& errs,
                                                   const FuncVerifier& v);
        static void ExpectStructDefinitionErrors(const std::string& snippet, const std::vector<ExpectedError>& errs,
                                                 const StructVerifier& v);
        static void ExpectEnumDefinitionErrors(const std::string& snippet, const std::vector<ExpectedError>& errs,
                                               const EnumVerifier& v);
        static void ExpectTypeAliasErrors(const std::string& snippet, const std::vector<ExpectedError>& errs,
                                          const AliasVerifier& v);
        static void ExpectExpressionErrors(const std::string& snippet, const std::vector<ExpectedError>& errs,
                                           const ExprVerifier& v);
        static void ExpectTypeAnnotationErrors(const std::string& snippet, const std::vector<ExpectedError>& errs,
                                               const TypeVerifier& v);
        static void ExpectModifierErrors(const std::string& snippet, const std::vector<ExpectedError>& errs,
                                         const ModifierVerifier& v);
        static void ExpectReturnErrors(const std::string& snippet, const std::vector<ExpectedError>& errs,
                                       const ReturnVerifier& v);

        template <typename Verifier>
        static void ExpectValidUnified(InjectableType type, const std::string& snippet,
                                       const Verifier& verifier, const std::string& group_name)
        {
            expand_to_top_level_stream(
                {type, snippet, UniversalVerifier(verifier), group_name, "", 0, 0},
                [&](ProcessingItem&& item)
                {
                    ProgramSpec spec;
                    std::visit([&](auto&& ver) { SpecAdder::add(spec, ver); }, item.verifier);

                    SCOPED_TRACE("Context: " + item.path_name);
                    ExpectValidParse(item.code, spec);
                },
                false
            );
        }

        template <typename Verifier>
        static void ExpectParseErrorsUnified(InjectableType type, const std::string& snippet,
                                             const std::vector<ExpectedError>& errors,
                                             const Verifier& verifier, const std::string& group_name)
        {
            auto* test_info = testing::UnitTest::GetInstance()->current_test_info();
            std::string test_name = test_info ? test_info->name() : "fallback";
            size_t base_seed = std::hash<std::string>{}(test_name);

            size_t scenario_index = 0;

            expand_to_top_level_stream(
                {type, snippet, UniversalVerifier(verifier), group_name, "", 0, 0},
                [&](ProcessingItem&& item)
                {
                    RunRecoveryScenario(std::move(item), errors, base_seed + (scenario_index++ * 2));
                },
                true
            );
        }
    };
}
