#pragma once

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <algorithm>
#include <iomanip>

#include "context_registry.h"
#include "error_registry.h"
#include "error_shifter.h"
#include "recovery_sentinel.h"
#include "frontend/lexer/lexer_stage.h"
#include "frontend/parser/parser_stage.h"
#include "core/compiler_context.h"
#include "frontend/parser/helpers/node_matchers.h"

#include "utils/parametrised_test_name_helper.h"

namespace valuascript::compiler::test
{
    struct ProcessingItem
    {
        InjectableType type;
        std::string code;
        UniversalVerifier verifier;
        std::string path_name;
        std::string cumulative_prefix;
        int depth;
    };

    struct RecoveryScenario
    {
        std::string path_name;
        std::string full_code;
        ProgramSpec spec;
        std::vector<ExpectedError> shifted_errors;
        int depth;
    };

    class ParserTestBase : public testing::Test
    {
    protected:
        static std::string format_source_with_lines(const std::string& code)
        {
            std::ostringstream oss;
            oss << "\n--- Full Source Code Listing ---\n";
            std::istringstream stream(code);
            std::string line;
            int line_num = 1;
            while (std::getline(stream, line))
            {
                oss << std::setw(3) << line_num++ << " | " << line << "\n";
            }
            oss << "--------------------------------\n";
            return oss.str();
        }

        static std::shared_ptr<Program> run_parser(const std::string& code, CompilerContext& context)
        {
            std::vector<CompilerStageArtifact> initial_artifacts = {
                {CompilerStageArtifactCode::SourceCode, code},
                {CompilerStageArtifactCode::FilePath, std::string("test_script.vs")}
            };

            LexerStage lexer;
            ParserStage parser;

            auto lexer_artifacts = initial_artifacts;
            lexer_artifacts.push_back(lexer.run(context, initial_artifacts));

            CompilerStageArtifact ast_artifact = parser.run(context, lexer_artifacts);

            return extract_artifact_data<std::shared_ptr<Program>>({ast_artifact}, CompilerStageArtifactCode::Ast);
        }

        static std::vector<ProcessingItem> expand_to_top_level(InjectableType start_type, const std::string& snippet,
                                                               const UniversalVerifier& verifier,
                                                               const std::string& base_name,
                                                               bool inject_sentinels = false)
        {
            std::vector<ProcessingItem> queue = {{start_type, snippet, verifier, base_name, "", 0}};
            std::vector<ProcessingItem> top_levels;

            while (!queue.empty())
            {
                auto item = queue.front();
                queue.erase(queue.begin());

                if (item.type == InjectableType::TopLevel ||
                    item.type == InjectableType::Import ||
                    item.type == InjectableType::Directive ||
                    item.type == InjectableType::Function ||
                    item.type == InjectableType::Struct ||
                    item.type == InjectableType::Enum ||
                    item.type == InjectableType::TypeAlias)
                {
                    top_levels.push_back(item);
                    continue;
                }

                if (item.type == InjectableType::StrongStatement)
                {
                    top_levels.push_back({
                        InjectableType::TopLevel,
                        item.code,
                        item.verifier,
                        item.path_name + " -> TopLevelPromotion",
                        item.cumulative_prefix,
                        item.depth + 1
                    });
                }

                if (item.depth >= 5) continue;

                auto contexts = ContextRegistry::get_all_for(item.type);
                for (const auto& ctx : contexts)
                {
                    bool is_recursive = (item.type == ctx.output_type);
                    if (is_recursive && item.path_name.find("Recurse") != std::string::npos) continue;

                    std::string step_name = is_recursive ? "Recurse(" + ctx.name + ")" : ctx.name;

                    if (ctx.is_block_context)
                    {
                        std::vector<RecoveryBlock> pre;
                        std::vector<RecoveryBlock> post;
                        std::string inner_code = item.code;
                        std::string inner_prefix = item.cumulative_prefix;

                        if (inject_sentinels)
                        {
                            size_t seed = std::hash<std::string>{}(item.path_name + ctx.name);
                            pre.push_back(RecoverySentinel::generate_block_sentinel(seed));
                            post.push_back(RecoverySentinel::generate_block_sentinel(seed + 1));

                            inner_code = pre[0].source + "\n  " + inner_code + "\n  " + post[0].source;
                            inner_prefix = pre[0].source + "\n  " + inner_prefix;
                        }

                        queue.push_back({
                            ctx.output_type,
                            ctx.prefix + inner_code + ctx.suffix,
                            ctx.transform_verifier_block(item.verifier, pre, post),
                            item.path_name + " -> " + step_name,
                            ctx.prefix + inner_prefix,
                            item.depth + 1
                        });
                    }
                    else
                    {
                        queue.push_back({
                            ctx.output_type,
                            ctx.prefix + item.code + ctx.suffix,
                            ctx.transform_verifier(item.verifier),
                            item.path_name + " -> " + step_name,
                            ctx.prefix + item.cumulative_prefix,
                            item.depth + 1
                        });
                    }
                }
            }
            return top_levels;
        }

        static std::vector<RecoveryScenario> generate_recovery_scenarios(
            InjectableType type,
            const std::string& snippet,
            const std::vector<ExpectedError>& expected_errors,
            const UniversalVerifier& verifier,
            const std::string& label,
            size_t base_seed)
        {
            auto top_levels = expand_to_top_level(type, snippet, verifier, label, true);
            std::vector<RecoveryScenario> scenarios;

            size_t ctx_index = 0;
            for (const auto& item : top_levels)
            {
                size_t current_seed = base_seed + ctx_index;
                ctx_index += 2;

                RecoveryBlock pre_sentinel = RecoverySentinel::generate_top_level_sentinel(current_seed);
                RecoveryBlock post_sentinel = RecoverySentinel::generate_top_level_sentinel(current_seed + 1);

                std::string middle_part = item.code;
                while (!middle_part.empty() && middle_part.back() == '\n') middle_part.pop_back();

                std::string full_code = pre_sentinel.source + "\n\n" + middle_part + "\n\n" + post_sentinel.source +
                    "\n";
                std::string prefix_for_shifting = pre_sentinel.source + "\n\n" + item.cumulative_prefix;

                ProgramSpec full_spec;
                if (pre_sentinel.add_to_spec) pre_sentinel.add_to_spec(full_spec);
                std::visit([&](auto&& ver) { SpecAdder::add(full_spec, ver); }, item.verifier);
                if (post_sentinel.add_to_spec) post_sentinel.add_to_spec(full_spec);

                auto shifted_errors = ErrorShifter::shift_errors(prefix_for_shifting, expected_errors);

                scenarios.push_back({
                    item.path_name,
                    full_code,
                    std::move(full_spec),
                    std::move(shifted_errors),
                    item.depth
                });
            }
            return scenarios;
        }

        static void ExpectValidParse(const std::string& code, const ProgramSpec& spec)
        {
            SCOPED_TRACE(format_source_with_lines(code));

            CompilerContext context;
            context.settings.fail_fast = false;

            std::shared_ptr<Program> ast;
            ASSERT_NO_THROW({
                ast = run_parser(code, context);
                }) << "Parser crashed unexpectedly on valid code.";

            const auto& errors = context.diagnostics.get_errors();
            if (!errors.empty())
            {
                ADD_FAILURE() << "Expected no errors, but got " << errors.size() << ". First error: " << errors[0].
                    what();
            }

            ASSERT_NE(ast, nullptr) << "Parser returned null AST.";
            ExpectProgram(ast.get(), spec);
        }

        template <typename Verifier>
        static void ExpectValidUnified(InjectableType type, const std::string& snippet, const Verifier& verifier,
                                       const std::string& context_group_name)
        {
            auto top_levels = expand_to_top_level(type, snippet, UniversalVerifier(verifier), context_group_name,
                                                  false);

            if (top_levels.empty())
            {
                ADD_FAILURE() << "No valid top-level expansions generated for InjectableType: " << static_cast<int>(
                    type);
                return;
            }

            for (const auto& item : top_levels)
            {
                ProgramSpec spec;
                std::visit([&](auto&& ver) { SpecAdder::add(spec, ver); }, item.verifier);

                SCOPED_TRACE("Testing " + context_group_name + " in context: " + item.path_name);
                ExpectValidParse(item.code, spec);
            }
        }

        static void ExpectParseErrors(const std::string& code, const std::vector<ExpectedError>& expected_errors,
                                      const std::optional<ProgramSpec>& spec = std::nullopt)
        {
            SCOPED_TRACE(format_source_with_lines(code));

            CompilerContext context;
            context.settings.fail_fast = false;

            std::shared_ptr<Program> ast;
            ASSERT_NO_THROW({
                ast = run_parser(code, context);
                }) << "Parser crashed unexpectedly on error recovery.";

            const auto& actual_errors = context.diagnostics.get_errors();
            ASSERT_EQ(actual_errors.size(), expected_errors.size())
                << "Mismatch in the number of collected errors.\n"
                << "Expected " << expected_errors.size() << ", but got " << actual_errors.size();

            size_t errors_to_check = std::min(actual_errors.size(), expected_errors.size());
            for (size_t i = 0; i < errors_to_check; ++i)
            {
                const auto& actual = actual_errors[i];
                const auto& expected = expected_errors[i];

                EXPECT_EQ(actual.get_code(), expected.code)
                    << "Error[" << i << "] Code mismatch.\nExpected Code: " << static_cast<int>(expected.code)
                    << "\nActual Code: " << static_cast<int>(actual.get_code())
                    << "\nActual Message: " << actual.what();

                if (!expected.skip_span_check)
                {
                    EXPECT_EQ(actual.get_span().line_start, expected.line_start)
                    << "Error[" << i << "] Line mismatch for error: " << actual.what();

                    EXPECT_EQ(actual.get_span().column_start, expected.column_start)
                        << "Error[" << i << "] Column mismatch for error: " << actual.what();

                    if (expected.line_end != 0)
                    {
                        EXPECT_EQ(actual.get_span().line_end, expected.line_end)
                            << "Error [" << i << "] End line mismatch for error: " << actual.what();
                    }

                    if (expected.column_end != 0)
                    {
                        EXPECT_EQ(actual.get_span().column_end, expected.column_end)
                            << "Error[" << i << "] End column mismatch for error: " << actual.what();
                    }
                }
            }

            if (spec.has_value())
            {
                ASSERT_NE(ast, nullptr) << "Parser returned null AST but a partial AST was expected.";
                ExpectProgram(ast.get(), spec.value());
            }
        }

        template <typename Verifier>
        static void ExpectParseErrorsUnified(InjectableType type,
                                             const std::string& snippet,
                                             const std::vector<ExpectedError>& expected_errors,
                                             const Verifier& verifier,
                                             const std::string& context_group_name)
        {
            auto* test_info = testing::UnitTest::GetInstance()->current_test_info();
            std::string full_test_name = test_info
                                             ? std::string(test_info->test_suite_name()) + "." + test_info->name()
                                             : "fallback_test_name";
            size_t base_seed = std::hash<std::string>{}(full_test_name);

            auto scenarios = generate_recovery_scenarios(
                type, snippet, expected_errors, UniversalVerifier(verifier), context_group_name, base_seed);

            for (const auto& sc : scenarios)
            {
                SCOPED_TRACE("Testing " + context_group_name + " Error Recovery in path: " + sc.path_name);
                ExpectParseErrors(sc.full_code, sc.shifted_errors, sc.spec);
            }
        }

        static void ExpectParseErrorsWithRecovery(const std::string& code,
                                                  const std::vector<ExpectedError>& expected_errors,
                                                  ProgramSpec broken_part_spec)
        {
            auto* test_info = testing::UnitTest::GetInstance()->current_test_info();
            std::string full_test_name = test_info
                                             ? std::string(test_info->test_suite_name()) + "." + test_info->name()
                                             : "fallback_test_name";

            size_t base_seed = std::hash<std::string>{}(full_test_name);

            RecoveryBlock pre_sentinel = RecoverySentinel::generate_top_level_sentinel(base_seed);
            RecoveryBlock post_sentinel = RecoverySentinel::generate_top_level_sentinel(base_seed + 1);

            std::string clean_code = code;
            while (!clean_code.empty() && clean_code.back() == '\n')
            {
                clean_code.pop_back();
            }

            std::string full_code = pre_sentinel.source + "\n\n" + clean_code + "\n\n" + post_sentinel.source + "\n";

            std::string prefix_for_shifting = pre_sentinel.source + "\n\n";
            auto shifted_errors = ErrorShifter::shift_errors(prefix_for_shifting, expected_errors);

            ProgramSpec full_spec;
            if (pre_sentinel.add_to_spec) pre_sentinel.add_to_spec(full_spec);
            full_spec = MergeSpecs(std::move(full_spec), std::move(broken_part_spec));
            if (post_sentinel.add_to_spec) post_sentinel.add_to_spec(full_spec);

            ExpectParseErrors(full_code, shifted_errors, full_spec);
        }

        static void ExpectValidAssignment(const std::string& code_snippet, const AssignmentVerifier& verifier)
        {
            ExpectValidUnified(InjectableType::StrongStatement, code_snippet, StmtVerifier(verifier), "Assignment");
        }

        static void ExpectValidReassignment(const std::string& code_snippet, const ReassignmentVerifier& verifier)
        {
            ExpectValidUnified(InjectableType::StrongStatement, code_snippet, StmtVerifier(verifier), "Reassignment");
        }

        static void ExpectValidExpressionStatement(const std::string& code_snippet, const ExprStmtVerifier& verifier)
        {
            ExpectValidUnified(InjectableType::StrongStatement, code_snippet, StmtVerifier(verifier),
                               "Expression Statement");
        }

        static void ExpectValidImport(const std::string& code_snippet, const ImportVerifier& verifier)
        {
            ExpectValidUnified(InjectableType::Import, code_snippet, verifier, "Import");
        }

        static void ExpectValidDirective(const std::string& code_snippet, const DirectiveVerifier& verifier)
        {
            ExpectValidUnified(InjectableType::Directive, code_snippet, verifier, "Directive");
        }

        static void ExpectValidFunctionDefinition(const std::string& code_snippet, const FuncVerifier& verifier)
        {
            ExpectValidUnified(InjectableType::Function, code_snippet, verifier, "Function");
        }

        static void ExpectValidStructDefinition(const std::string& code_snippet, const StructVerifier& verifier)
        {
            ExpectValidUnified(InjectableType::Struct, code_snippet, verifier, "Struct");
        }

        static void ExpectValidEnumDefinition(const std::string& code_snippet, const EnumVerifier& verifier)
        {
            ExpectValidUnified(InjectableType::Enum, code_snippet, verifier, "Enum");
        }

        static void ExpectValidTypeAlias(const std::string& code_snippet, const AliasVerifier& verifier)
        {
            ExpectValidUnified(InjectableType::TypeAlias, code_snippet, verifier, "Type Alias");
        }

        static void ExpectValidExpression(const std::string& code_snippet, const ExprVerifier& verifier)
        {
            ExpectValidUnified(InjectableType::Expression, code_snippet, verifier, "Expression");
        }

        static void ExpectValidTypeAnnotation(const std::string& code_snippet, const TypeVerifier& verifier)
        {
            ExpectValidUnified(InjectableType::TypeAnnotation, code_snippet, verifier, "Type Annotation");
        }

        static void ExpectValidModifiers(const std::string& code_snippet, const ModifierVerifier& verifier)
        {
            ExpectValidUnified(InjectableType::Modifier, code_snippet, verifier, "Modifier");
        }

        static void ExpectValidReturn(const std::string& code_snippet, const ReturnVerifier& verifier)
        {
            ExpectValidUnified(InjectableType::WeakStatement, code_snippet, verifier, "Return");
        }

        static void ExpectAssignmentErrors(const std::string& snippet,
                                           const std::vector<ExpectedError>& expected_errors,
                                           const AssignmentVerifier& verifier)
        {
            ExpectParseErrorsUnified(InjectableType::StrongStatement, snippet, expected_errors, StmtVerifier(verifier),
                                     "Assignment");
        }

        static void ExpectReassignmentErrors(const std::string& snippet,
                                             const std::vector<ExpectedError>& expected_errors,
                                             const ReassignmentVerifier& verifier)
        {
            ExpectParseErrorsUnified(InjectableType::StrongStatement, snippet, expected_errors, StmtVerifier(verifier),
                                     "Reassignment");
        }

        static void ExpectExpressionStatementErrors(const std::string& snippet,
                                                    const std::vector<ExpectedError>& expected_errors,
                                                    const ExprStmtVerifier& verifier)
        {
            ExpectParseErrorsUnified(InjectableType::StrongStatement, snippet, expected_errors, StmtVerifier(verifier),
                                     "Expression Statement");
        }

        static void ExpectImportErrors(const std::string& snippet, const std::vector<ExpectedError>& expected_errors,
                                       const ImportVerifier& verifier)
        {
            ExpectParseErrorsUnified(InjectableType::Import, snippet, expected_errors, verifier, "Import");
        }

        static void ExpectDirectiveErrors(const std::string& snippet, const std::vector<ExpectedError>& expected_errors,
                                          const DirectiveVerifier& verifier)
        {
            ExpectParseErrorsUnified(InjectableType::Directive, snippet, expected_errors, verifier, "Directive");
        }

        static void ExpectFunctionDefinitionErrors(const std::string& snippet,
                                                   const std::vector<ExpectedError>& expected_errors,
                                                   const FuncVerifier& verifier)
        {
            ExpectParseErrorsUnified(InjectableType::Function, snippet, expected_errors, verifier,
                                     "Function Definition");
        }

        static void ExpectStructDefinitionErrors(const std::string& snippet,
                                                 const std::vector<ExpectedError>& expected_errors,
                                                 const StructVerifier& verifier)
        {
            ExpectParseErrorsUnified(InjectableType::Struct, snippet, expected_errors, verifier, "Struct Definition");
        }

        static void ExpectEnumDefinitionErrors(const std::string& snippet,
                                               const std::vector<ExpectedError>& expected_errors,
                                               const EnumVerifier& verifier)
        {
            ExpectParseErrorsUnified(InjectableType::Enum, snippet, expected_errors, verifier, "Enum Definition");
        }

        static void ExpectTypeAliasErrors(const std::string& snippet, const std::vector<ExpectedError>& expected_errors,
                                          const AliasVerifier& verifier)
        {
            ExpectParseErrorsUnified(InjectableType::TypeAlias, snippet, expected_errors, verifier, "Type Alias");
        }

        static void ExpectExpressionErrors(const std::string& snippet,
                                           const std::vector<ExpectedError>& expected_errors,
                                           const ExprVerifier& verifier)
        {
            ExpectParseErrorsUnified(InjectableType::Expression, snippet, expected_errors, verifier, "Expression");
        }

        static void ExpectTypeAnnotationErrors(const std::string& snippet,
                                               const std::vector<ExpectedError>& expected_errors,
                                               const TypeVerifier& verifier)
        {
            ExpectParseErrorsUnified(InjectableType::TypeAnnotation, snippet, expected_errors, verifier,
                                     "Type Annotation");
        }

        static void ExpectModifierErrors(const std::string& snippet, const std::vector<ExpectedError>& expected_errors,
                                         const ModifierVerifier& verifier)
        {
            ExpectParseErrorsUnified(InjectableType::Modifier, snippet, expected_errors, verifier, "Modifier");
        }

        static void ExpectReturnErrors(const std::string& snippet, const std::vector<ExpectedError>& expected_errors,
                                       const ReturnVerifier& verifier)
        {
            ExpectParseErrorsUnified(InjectableType::WeakStatement, snippet, expected_errors, verifier, "Return");
        }
    };
}
