#pragma once

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <algorithm>
#include <iomanip>

#include "assignment_contexts_provider.h"
#include "recovery_sentinel.h"
#include "frontend/lexer/lexer_stage.h"
#include "frontend/parser/parser_stage.h"
#include "core/compiler_context.h"
#include "core/valuascript_exception.h"
#include "frontend/parser/helpers/node_matchers.h"
#include "expression_contexts_provider.h"
#include "expression_statement_contexts_provider.h"
#include "modifier_contexts_provider.h"
#include "reassignment_contexts_provider.h"
#include "return_statement_contexts_provider.h"
#include "type_annotation_contexts_provider.h"

namespace valuascript::compiler::test
{
    struct ExpectedError
    {
        ValuascriptErrorCode code;
        size_t line_start;
        size_t column_start;
        size_t line_end;
        size_t column_end;

        ExpectedError(ValuascriptErrorCode code,
                      size_t line_start,
                      size_t column_start,
                      size_t line_end = 0,
                      size_t column_end = 0)
            : code(code),
              line_start(line_start),
              column_start(column_start),
              line_end(line_end),
              column_end(column_end)
        {
        }
    };

    struct ValidParserTestCase
    {
        std::string test_name;
        std::string source_code;
        ProgramSpec expected_ast;
    };

    struct ErrorParserTestCase
    {
        std::string test_name;
        std::string source_code;
        std::vector<ExpectedError> expected_errors;
        std::optional<ProgramSpec> expected_ast = std::nullopt;
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

            CompilerStageArtifact ast_artifact;
            ast_artifact = parser.run(context, lexer_artifacts);

            return std::any_cast<std::shared_ptr<Program>>(ast_artifact.data);
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

        static void run_valid_parser_test(const ValidParserTestCase& test_case)
        {
            ExpectValidParse(test_case.source_code, test_case.expected_ast);
        }

        static void ExpectValidAssignment(const std::string& assign_code, const StmtVerifier& assign_verifier)
        {
            for (const auto& ctx : AssignmentContextsProvider::get_all())
            {
                std::string code = AssignmentContextsProvider::inject(ctx.source_template, assign_code);

                ProgramSpec spec;
                ctx.add_to_spec(spec, assign_verifier);

                SCOPED_TRACE("Testing assignment context: " + ctx.name);
                ExpectValidParse(code, spec);
            }
        }

        static void ExpectValidReassignment(const std::string& reassign_code, const StmtVerifier& reassign_verifier)
        {
            for (const auto& ctx : ReassignmentContextsProvider::get_all())
            {
                std::string code = ReassignmentContextsProvider::inject(ctx.source_template, reassign_code);

                ProgramSpec spec;
                ctx.add_to_spec(spec, reassign_verifier);

                SCOPED_TRACE("Testing reassignment context: " + ctx.name);
                ExpectValidParse(code, spec);
            }
        }

        static void ExpectValidExpressionStatement(const std::string& stmt_code, const StmtVerifier& stmt_verifier)
        {
            for (const auto& ctx : ExpressionStatementContextsProvider::get_all())
            {
                std::string code = ExpressionStatementContextsProvider::inject(ctx.source_template, stmt_code);

                ProgramSpec spec;
                ctx.add_to_spec(spec, stmt_verifier);

                SCOPED_TRACE("Testing expression statement context: " + ctx.name);
                ExpectValidParse(code, spec);
            }
        }

        static void ExpectValidExpression(const std::string& expr_code, const ExprVerifier& expr_verifier)
        {
            for (const auto& ctx : ExpressionContextsProvider::get_all())
            {
                std::string code = ExpressionContextsProvider::inject(ctx.source_template, expr_code);

                ProgramSpec spec;
                ctx.add_to_spec(spec, expr_verifier);

                SCOPED_TRACE("Testing expression context: " + ctx.name);

                ExpectValidParse(code, spec);
            }
        }

        static void ExpectValidTypeAnnotation(const std::string& type_code, const TypeVerifier& type_verifier)
        {
            for (const auto& ctx : TypeAnnotationContextsProvider::get_all())
            {
                std::string code = TypeAnnotationContextsProvider::inject(ctx.source_template, type_code);

                ProgramSpec spec;
                ctx.add_to_spec(spec, type_verifier);

                SCOPED_TRACE("Testing type annotation context: " + ctx.name);
                ExpectValidParse(code, spec);
            }
        }

        static void ExpectValidModifiers(const std::string& mods_code, const std::vector<ModifierSpec>& expected_mods)
        {
            for (const auto& ctx : ModifierContextsProvider::get_all())
            {
                std::string code = ModifierContextsProvider::inject(ctx.source_template, mods_code + " ");

                ProgramSpec spec;
                ctx.add_to_spec(spec, expected_mods);

                SCOPED_TRACE("Testing modifier context: " + ctx.name);
                ExpectValidParse(code, spec);
            }
        }

        static void ExpectValidReturn(const std::string& ret_code, const StmtVerifier& ret_verifier)
        {
            for (const auto& ctx : ReturnStatementContextsProvider::get_all())
            {
                std::string code = ReturnStatementContextsProvider::inject(ctx.source_template, ret_code);

                ProgramSpec spec;
                ctx.add_to_spec(spec, ret_verifier);

                SCOPED_TRACE("Testing return context: " + ctx.name);
                ExpectValidParse(code, spec);
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
                    << "Error [" << i << "] Code mismatch.\nExpected Code: " << static_cast<int>(expected.code)
                    << "\nActual Code: " << static_cast<int>(actual.get_code())
                    << "\nActual Message: " << actual.what();

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

            if (spec.has_value())
            {
                ASSERT_NE(ast, nullptr) << "Parser returned null AST but a partial AST was expected.";
                ExpectProgram(ast.get(), spec.value());
            }
        }

        static void ExpectParseErrorsWithRecovery(const std::string& code,
                                                  const std::vector<ExpectedError>& expected_errors,
                                                  ProgramSpec broken_part_spec)
        {
            auto* test_info = testing::UnitTest::GetInstance()->current_test_info();
            std::string full_test_name = std::string(test_info->test_suite_name()) + "." + test_info->name();

            size_t rotation_index = std::hash<std::string>{}(full_test_name);

            auto recovery = RecoverySentinel::generate(rotation_index);

            std::string full_code = code + recovery.source;
            ProgramSpec full_spec = MergeSpecs(std::move(broken_part_spec), std::move(recovery.spec));

            ExpectParseErrors(full_code, expected_errors, full_spec);
        }

        static void run_error_parser_test(const ErrorParserTestCase& test_case)
        {
            ExpectParseErrors(test_case.source_code, test_case.expected_errors, test_case.expected_ast);
        }
    };
}
