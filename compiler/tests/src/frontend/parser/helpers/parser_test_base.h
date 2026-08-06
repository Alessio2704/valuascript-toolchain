#pragma once

#include <gtest/gtest.h>

#include "deterministic_sampler.h"
#include "recovery_sentinel.h"
#include "test_structures.h"
#include "core/compiler_context.h"
#include "../expansion_and_sentinels/expansion_calculator.h"

#include "utils/parametrised_test_name_helper.h"
#include <string_view>

#include "frontend/parser/expansion_and_sentinels/expansion_policy.h"

namespace valuascript::compiler::test
{
    struct ConstructedRecoveryProgram
    {
        std::string full_code;
        ProgramSpec full_spec;
        std::string prefix_for_shifting;
        std::string path_name = "";
    };

    using ExpansionCallback = std::function<void(ProcessingItem&&)>;

    class ParserTestBase : public testing::Test
    {
    protected:
        static std::string format_source_with_lines(const std::string& code);
        static std::shared_ptr<Program> run_parser(const std::string& code, CompilerContext& context);

        static void expand_to_top_level_stream(std::vector<ProcessingItem> items,
                                               const ExpansionCallback& callback,
                                               bool inject_sentinels = false,
                                               std::optional<ExpansionPolicy> policy_override = std::nullopt);

        template <typename Verifier>
        static void expand_to_top_level_stream(InjectableType type,
                                               const std::string& snippet,
                                               const Verifier& verifier,
                                               const std::string& group_name,
                                               const ExpansionCallback& callback,
                                               bool inject_sentinels = false,
                                               const std::vector<std::string_view>& skip_contexts = {},
                                               const std::vector<ContextOverrideAny>& context_overrides = {},
                                               std::optional<ExpansionPolicy> policy_override = std::nullopt,
                                               const std::vector<SentinelKind>& excluded_sentinels = {},
                                               const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            auto items = apply_context_augmentations(type, snippet, UniversalVerifier(verifier), group_name, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
            expand_to_top_level_stream(std::move(items), callback, inject_sentinels, policy_override);
        }

        template <typename Verifier>
        static size_t get_augmentation_count(InjectableType type,
                                             const std::string& snippet,
                                             const Verifier& verifier,
                                             const std::string& group_name,
                                             const std::vector<std::string_view>& skip_contexts = {})
        {
            return apply_context_augmentations(type, snippet, UniversalVerifier(verifier), group_name, skip_contexts).size();
        }

    private:
        static std::vector<ProcessingItem> apply_context_augmentations(InjectableType type,
                                                                       const std::string& snippet,
                                                                       const UniversalVerifier& verifier,
                                                                       const std::string& group_name,
                                                                       const std::vector<std::string_view>&
                                                                           skip_contexts = {},
                                                                       const std::vector<ContextOverrideAny>&
                                                                           context_overrides = {},
                                                                       const std::vector<SentinelKind>&
                                                                           excluded_sentinels = {},
                                                                       const std::vector<SentinelKind>&
                                                                           accepted_sentinels = {});

    public:
        template <typename Callback>
        static void ForEachRecoveryProgram(const ProcessingItem& item, size_t seed, Callback&& callback)
        {
            auto progs = BuildRecoveryPrograms(item, seed);
            for (const auto& prog : progs)
            {
                SCOPED_TRACE("Recovery Path: " + (prog.path_name.empty() ? item.path_name : prog.path_name));
                callback(prog);
            }
        }

        template <typename Callback>
        static void ForEachRecoveryProgram(const ProcessingItem& item, ProgramSpec inner_spec, size_t seed, Callback&& callback)
        {
            auto progs = BuildRecoveryPrograms(item, seed, std::move(inner_spec));
            for (const auto& prog : progs)
            {
                SCOPED_TRACE("Recovery Path: " + (prog.path_name.empty() ? item.path_name : prog.path_name));
                callback(prog);
            }
        }

        template <typename Callback>
        static void ForEachRecoveryProgram(std::string inner_code,
                                           ProgramSpec inner_spec,
                                           const std::string& inner_prefix,
                                           size_t seed,
                                           const std::vector<SentinelKind>& excluded_sentinels,
                                           const std::vector<SentinelKind>& accepted_sentinels,
                                           const std::string& path_name,
                                           Callback&& callback)
        {
            auto progs = BuildRecoveryPrograms(std::move(inner_code), std::move(inner_spec), inner_prefix, seed, excluded_sentinels, accepted_sentinels, path_name);
            for (const auto& prog : progs)
            {
                if (!prog.path_name.empty())
                {
                    SCOPED_TRACE("Recovery Path: " + prog.path_name);
                }
                callback(prog);
            }
        }

        template <typename Callback>
        static void ForEachRecoveryProgram(std::string inner_code,
                                           ProgramSpec inner_spec,
                                           const std::string& inner_prefix,
                                           size_t seed,
                                           Callback&& callback)
        {
            ForEachRecoveryProgram(std::move(inner_code), std::move(inner_spec), inner_prefix, seed, {}, {}, "", std::forward<Callback>(callback));
        }

    private:
        static std::vector<ConstructedRecoveryProgram> BuildRecoveryPrograms(std::string inner_code,
                                                                             ProgramSpec inner_spec,
                                                                             const std::string& inner_prefix,
                                                                             size_t seed,
                                                                             const std::vector<SentinelKind>& excluded_sentinels = {},
                                                                             const std::vector<SentinelKind>& accepted_sentinels = {},
                                                                             const std::string& path_name = "");

        static std::vector<ConstructedRecoveryProgram> BuildRecoveryPrograms(const ProcessingItem& item,
                                                                             size_t seed,
                                                                             std::optional<ProgramSpec> inner_spec_override = std::nullopt);

    protected:

        static void RunRecoveryScenario(ProcessingItem&& item, const std::vector<ParserExpectedError>& errors,
                                        size_t seed);

        static void ExpectValidParse(const std::string& code, const ProgramSpec& spec);
        static void ExpectParseErrors(const std::string& code, const std::vector<ParserExpectedError>& expected_errors,
                                      const std::optional<ProgramSpec>& spec = std::nullopt);
        static void ExpectParseErrorsWithRecovery(const std::string& code,
                                                  const std::vector<ParserExpectedError>& expected_errors,
                                                  ProgramSpec broken_part_spec);

        static void ExpectValidAssignment(const std::string& snippet, const AssignmentVerifier& verifier,
                                          const std::vector<std::string_view>& skip_contexts = {});
        static void ExpectValidReassignment(const std::string& snippet, const ReassignmentVerifier& verifier,
                                            const std::vector<std::string_view>& skip_contexts = {});
        static void ExpectValidExpressionStatement(const std::string& snippet, const ExprStmtVerifier& verifier,
                                                   const std::vector<std::string_view>& skip_contexts = {});
        static void ExpectValidImport(const std::string& snippet, const ImportVerifier& verifier,
                                      const std::vector<std::string_view>& skip_contexts = {});
        static void ExpectValidDirective(const std::string& snippet, const DirectiveVerifier& verifier,
                                         const std::vector<std::string_view>& skip_contexts = {});
        static void ExpectValidFunctionDefinition(const std::string& snippet, const FuncVerifier& verifier,
                                                  const std::vector<std::string_view>& skip_contexts = {});
        static void ExpectValidExtensionDefinition(const std::string& snippet, const ExtVerifier& verifier,
                                                  const std::vector<std::string_view>& skip_contexts = {});
        static void ExpectValidStructDefinition(const std::string& snippet, const StructVerifier& verifier,
                                                const std::vector<std::string_view>& skip_contexts = {});
        static void ExpectValidEnumDefinition(const std::string& snippet, const EnumVerifier& verifier,
                                              const std::vector<std::string_view>& skip_contexts = {});
        static void ExpectValidTypeAlias(const std::string& snippet, const AliasVerifier& verifier,
                                         const std::vector<std::string_view>& skip_contexts = {});
        static void ExpectValidExpression(const std::string& snippet, const ExprVerifier& verifier,
                                          const std::vector<std::string_view>& skip_contexts = {});
        static void ExpectValidTypeAnnotation(const std::string& snippet, const TypeVerifier& verifier,
                                              const std::vector<std::string_view>& skip_contexts = {});
        static void ExpectValidModifiers(const std::string& snippet, const ModifierVerifier& verifier,
                                         const std::vector<std::string_view>& skip_contexts = {});
        static void ExpectValidReturn(const std::string& snippet, const ReturnVerifier& verifier,
                                      const std::vector<std::string_view>& skip_contexts = {});

        static void ExpectAssignmentErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                           const OneOf<AssignmentVerifier>& v,
                                           const std::vector<std::string_view>& skip_contexts = {},
                                           const std::vector<ContextOverride<AssignmentVerifier>>& context_overrides = {},
                                           const std::vector<SentinelKind>& excluded_sentinels = {},
                                           const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectReassignmentErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                             const OneOf<ReassignmentVerifier>& v,
                                             const std::vector<std::string_view>& skip_contexts = {},
                                             const std::vector<ContextOverride<ReassignmentVerifier>>& context_overrides = {},
                                             const std::vector<SentinelKind>& excluded_sentinels = {},
                                             const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectExpressionStatementErrors(const std::string& snippet,
                                                    const std::vector<ParserExpectedError>& errs,
                                                    const OneOf<ExprStmtVerifier>& v,
                                                    const std::vector<std::string_view>& skip_contexts = {},
                                                    const std::vector<ContextOverride<ExprStmtVerifier>>& context_overrides = {},
                                                    const std::vector<SentinelKind>& excluded_sentinels = {},
                                                    const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectImportErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                       const OneOf<ImportVerifier>& v,
                                       const std::vector<std::string_view>& skip_contexts = {},
                                       const std::vector<ContextOverride<ImportVerifier>>& context_overrides = {},
                                       const std::vector<SentinelKind>& excluded_sentinels = {},
                                       const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectDirectiveErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                          const OneOf<DirectiveVerifier>& v,
                                          const std::vector<std::string_view>& skip_contexts = {},
                                          const std::vector<ContextOverride<DirectiveVerifier>>& context_overrides = {},
                                          const std::vector<SentinelKind>& excluded_sentinels = {},
                                          const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectFunctionDefinitionErrors(const std::string& snippet,
                                                   const std::vector<ParserExpectedError>& errs,
                                                   const OneOf<FuncVerifier>& v,
                                                   const std::vector<std::string_view>& skip_contexts = {},
                                                   const std::vector<ContextOverride<FuncVerifier>>& context_overrides = {},
                                                   const std::vector<SentinelKind>& excluded_sentinels = {},
                                                   const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectExtensionDefinitionErrors(const std::string& snippet,
                                                    const std::vector<ParserExpectedError>& errs,
                                                    const OneOf<ExtVerifier>& v,
                                                    const std::vector<std::string_view>& skip_contexts = {},
                                                    const std::vector<ContextOverride<ExtVerifier>>& context_overrides = {},
                                                    const std::vector<SentinelKind>& excluded_sentinels = {},
                                                    const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectStructDefinitionErrors(const std::string& snippet,
                                                 const std::vector<ParserExpectedError>& errs,
                                                 const OneOf<StructVerifier>& v,
                                                 const std::vector<std::string_view>& skip_contexts = {},
                                                 const std::vector<ContextOverride<StructVerifier>>& context_overrides = {},
                                                 const std::vector<SentinelKind>& excluded_sentinels = {},
                                                 const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectEnumDefinitionErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                               const OneOf<EnumVerifier>& v,
                                               const std::vector<std::string_view>& skip_contexts = {},
                                               const std::vector<ContextOverride<EnumVerifier>>& context_overrides = {},
                                               const std::vector<SentinelKind>& excluded_sentinels = {},
                                               const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectTypeAliasErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                          const OneOf<AliasVerifier>& v,
                                          const std::vector<std::string_view>& skip_contexts = {},
                                          const std::vector<ContextOverride<AliasVerifier>>& context_overrides = {},
                                          const std::vector<SentinelKind>& excluded_sentinels = {},
                                          const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectExpressionErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                           const OneOf<ExprVerifier>& v,
                                           const std::vector<std::string_view>& skip_contexts = {},
                                           const std::vector<ContextOverride<ExprVerifier>>& context_overrides = {},
                                           const std::vector<SentinelKind>& excluded_sentinels = {},
                                           const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectTypeAnnotationErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                               const OneOf<TypeVerifier>& v,
                                               const std::vector<std::string_view>& skip_contexts = {},
                                               const std::vector<ContextOverride<TypeVerifier>>& context_overrides = {},
                                               const std::vector<SentinelKind>& excluded_sentinels = {},
                                               const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectModifierErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                         const OneOf<ModifierVerifier>& v,
                                         const std::vector<std::string_view>& skip_contexts = {},
                                         const std::vector<ContextOverride<ModifierVerifier>>& context_overrides = {},
                                         const std::vector<SentinelKind>& excluded_sentinels = {},
                                         const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectReturnErrors(const std::string& snippet, const std::vector<ParserExpectedError>& errs,
                                       const OneOf<ReturnVerifier>& v,
                                       const std::vector<std::string_view>& skip_contexts = {},
                                       const std::vector<ContextOverride<ReturnVerifier>>& context_overrides = {},
                                       const std::vector<SentinelKind>& excluded_sentinels = {},
                                       const std::vector<SentinelKind>& accepted_sentinels = {});

        static void ExpectValidUnified(InjectableType type, std::vector<ProcessingItem> items,
                                       const std::string& group_name);

        template <typename Verifier>
        static void ExpectValidUnified(InjectableType type, const std::string& snippet,
                                       const Verifier& verifier, const std::string& group_name,
                                       const std::vector<std::string_view>& skip_contexts = {})
        {
            auto items = apply_context_augmentations(type, snippet, verifier, group_name, skip_contexts);
            ExpectValidUnified(type, std::move(items), group_name);
        }

        static void ExpectParseErrorsUnified(InjectableType type, std::vector<ProcessingItem> items,
                                             const std::vector<ParserExpectedError>& errors,
                                             const std::string& group_name);

        template <typename Verifier>
        static void ExpectParseErrorsUnified(InjectableType type, const std::string& snippet,
                                             const std::vector<ParserExpectedError>& errors,
                                             const Verifier& verifier, const std::string& group_name,
                                             const std::vector<std::string_view>& skip_contexts = {},
                                             const std::vector<ContextOverrideAny>& context_overrides = {},
                                             const std::vector<SentinelKind>& excluded_sentinels = {},
                                             const std::vector<SentinelKind>& accepted_sentinels = {})
        {
            auto items = apply_context_augmentations(type, snippet, UniversalVerifier(verifier), group_name, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
            ExpectParseErrorsUnified(type, std::move(items), errors, group_name);
        }
    };
}
