#include "parser_runner.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <set>
#include "frontend/lexer/lexer_stage.h"
#include "frontend/parser/parser_stage.h"
#include "error_shifter.h"
#include "recovery_program_builder.h"
#include "deterministic_sampler.h"
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    std::string ParserRunner::format_source_with_lines(const std::string& code)
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

    std::string ParserRunner::format_affected_source_snippet(const std::string& code, const std::vector<size_t>& error_lines, int context_lines)
    {
        std::istringstream stream(code);
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(stream, line))
        {
            lines.push_back(line);
        }

        if (lines.size() <= 40)
        {
            return format_source_with_lines(code);
        }

        std::set<int> lines_to_show;
        if (error_lines.empty())
        {
            for (size_t i = 1; i <= std::min<size_t>(lines.size(), 30); ++i)
                lines_to_show.insert(static_cast<int>(i));
        }
        else
        {
            for (size_t err_line : error_lines)
            {
                int start = std::max(1, static_cast<int>(err_line) - context_lines);
                int end = std::min(static_cast<int>(lines.size()), static_cast<int>(err_line) + context_lines);
                for (int l = start; l <= end; ++l)
                {
                    lines_to_show.insert(l);
                }
            }
        }

        std::set<size_t> err_set(error_lines.begin(), error_lines.end());
        std::ostringstream oss;
        oss << "\n--- Affected Source Code Snippet ---\n";
        int prev_line = 0;
        for (int l : lines_to_show)
        {
            if (prev_line != 0 && l > prev_line + 1)
            {
                oss << "...\n";
            }
            prev_line = l;
            bool is_err = err_set.count(static_cast<size_t>(l)) > 0;
            oss << (is_err ? ">>> " : "    ") << std::setw(3) << l << " | " << lines[static_cast<size_t>(l - 1)] << "\n";
        }
        oss << "------------------------------------\n";
        return oss.str();
    }

    std::shared_ptr<Program> ParserRunner::run_parser(const std::string& code, CompilerContext& context)
    {
        thread_local LexerStage lexer;
        thread_local ParserStage parser;

        std::vector<CompilerStageArtifact> artifacts;
        artifacts.reserve(3);
        artifacts.emplace_back(CompilerStageArtifactCode::SourceCode, code);
        artifacts.emplace_back(CompilerStageArtifactCode::FilePath, std::string("test_script.vs"));

        artifacts.push_back(lexer.run(context, artifacts));

        CompilerStageArtifact ast_artifact = parser.run(context, artifacts);
        return extract_artifact_data<std::shared_ptr<Program>>({ast_artifact}, CompilerStageArtifactCode::Ast);
    }

    void ParserRunner::ExpectValidParse(const std::string& code, const ProgramSpec& spec)
    {
        CompilerContext context;
        context.settings.fail_fast = false;
        std::shared_ptr<Program> ast = run_parser(code, context);

        const auto& errors = context.diagnostics.get_errors();
        if (!errors.empty())
        {
            std::vector<size_t> error_lines;
            for (const auto& err : errors)
            {
                error_lines.push_back(err.get_span().line_start);
            }
            SCOPED_TRACE(format_affected_source_snippet(code, error_lines, 8));
            ADD_FAILURE() << "Expected no errors, but got " << errors.size() << ". First: " << errors[0].what();
        }

        ASSERT_NE(ast, nullptr) << "Parser returned null AST.";
        ExpectProgram(ast.get(), spec);
    }

    void ParserRunner::ExpectParseErrors(const std::string& code,
                                           const std::vector<ParserExpectedError>& expected_errors,
                                           const std::optional<ProgramSpec>& spec)
    {
        SCOPED_TRACE(format_source_with_lines(code));
        CompilerContext context;
        context.settings.fail_fast = false;
        std::shared_ptr<Program> ast = run_parser(code, context);

        const auto& actual_errors = context.diagnostics.get_errors();
        if (actual_errors.size() != expected_errors.size())
        {
            for (const auto& err : actual_errors)
            {
                std::cout << "ACTUAL ERR: code=" << err.get_code().index() << " msg=" << err.what() << " span=" << err.
                    get_span().line_start << ":" << err.get_span().column_start << "-" << err.get_span().line_end << ":"
                    << err.get_span().column_end << std::endl;
            }
        }
        ASSERT_EQ(actual_errors.size(), expected_errors.size()) << "Error count mismatch.";

        for (size_t i = 0; i < actual_errors.size(); ++i)
        {
            EXPECT_EQ(actual_errors[i].get_code(), expected_errors[i].code);
            if (!expected_errors[i].skip_span_check)
            {
                EXPECT_EQ(actual_errors[i].get_span().line_start, expected_errors[i].line_start) <<
                    "Line start mismatch on error: " << i << std::endl;
                EXPECT_EQ(actual_errors[i].get_span().line_end, expected_errors[i].line_end) <<
                    "Line end mismatch on error: " << i << std::endl;
                EXPECT_EQ(actual_errors[i].get_span().column_start, expected_errors[i].column_start) <<
                    "Column start mismatch on error: " << i << std::endl;
                EXPECT_EQ(actual_errors[i].get_span().column_end, expected_errors[i].column_end) <<
                    "Column end mismatch on error: " << i << std::endl;
            }
        }

        if (spec.has_value())
        {
            ASSERT_NE(ast, nullptr);
            ExpectProgram(ast.get(), spec.value());
        }
    }

    void ParserRunner::ExpectParseErrorsWithRecovery(const std::string& code,
                                                       const std::vector<ParserExpectedError>& expected_errors,
                                                       ProgramSpec broken_part_spec)
    {
        auto* test_info = testing::UnitTest::GetInstance()->current_test_info();
        size_t base_seed = DeterministicSampler::make_seed(test_info ? test_info->name() : "fallback");

        RecoveryProgramBuilder::ForEachRecoveryProgram(code, std::move(broken_part_spec), "", base_seed,
                                                        [&](const ConstructedRecoveryProgram& prog)
                                                        {
                                                            auto shifted = ErrorShifter::shift_errors(prog.prefix_for_shifting, expected_errors);
                                                            ExpectParseErrors(prog.full_code, shifted, prog.full_spec);
                                                        });
    }
}
