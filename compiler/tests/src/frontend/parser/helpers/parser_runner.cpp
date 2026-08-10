#include "parser_runner.h"
#include <sstream>
#include <iomanip>
#include <iostream>
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
        SCOPED_TRACE(format_source_with_lines(code));
        CompilerContext context;
        context.settings.fail_fast = false;
        std::shared_ptr<Program> ast = run_parser(code, context);

        const auto& errors = context.diagnostics.get_errors();
        if (!errors.empty())
        {
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
