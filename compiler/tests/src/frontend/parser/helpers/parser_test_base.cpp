#include "parser_test_base.h"
#include <sstream>
#include <iomanip>
#include "context_registry.h"
#include "error_shifter.h"
#include "recovery_sentinel.h"
#include "frontend/lexer/lexer_stage.h"
#include "frontend/parser/parser_stage.h"
#include "context_tree_walker.h"

namespace valuascript::compiler::test
{
    std::string ParserTestBase::format_source_with_lines(const std::string& code)
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

    std::shared_ptr<Program> ParserTestBase::run_parser(const std::string& code, CompilerContext& context)
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

    void ParserTestBase::expand_to_top_level_stream(ProcessingItem&& initial_item,
                                                    const ExpansionCallback& callback,
                                                    bool inject_sentinels)
    {
        ContextTreeWalker<ProcessingItem>::Callbacks cb;

        cb.get_type = [](const ProcessingItem& item) { return item.type; };

        cb.on_terminal = [&](ProcessingItem item)
        {
            callback(std::move(item));
        };

        cb.on_promotion = [&](const ProcessingItem& item)
        {
            callback({
                InjectableType::TopLevel, item.code, item.verifier,
                item.path_name + " -> TopLevelPromotion", item.cumulative_prefix,
                item.depth + 1, item.recursion_depth
            });
        };

        cb.on_normal_branch = [](const ProcessingItem& item, const Context& ctx, int next_rec_depth)
        {
            std::string next_path = item.path_name + " -> " +
                (next_rec_depth > item.recursion_depth ? ctx.name + "(Recurse)" : ctx.name);

            return ProcessingItem{
                ctx.output_type,
                ctx.prefix + item.code + ctx.suffix,
                ctx.transform_verifier(item.verifier),
                next_path,
                ctx.prefix + item.cumulative_prefix,
                item.depth + 1,
                next_rec_depth
            };
        };

        cb.on_block_branch = [inject_sentinels](const ProcessingItem& item, const Context& ctx, int next_rec_depth)
        {
            std::vector<RecoveryBlock> pre, post;
            std::string inner_code = item.code;
            std::string inner_prefix = item.cumulative_prefix;

            std::string next_path = item.path_name + " -> " +
                (next_rec_depth > item.recursion_depth ? ctx.name + "(Recurse)" : ctx.name);

            if (inject_sentinels)
            {
                size_t seed = std::hash<std::string>{}(item.path_name + ctx.name);
                pre.push_back(RecoverySentinel::generate_block_sentinel(seed));
                post.push_back(RecoverySentinel::generate_block_sentinel(seed + 1));
                inner_code = pre[0].source + "\n  " + inner_code + "\n  " + post[0].source;
                inner_prefix = pre[0].source + "\n  " + inner_prefix;
            }

            return ProcessingItem{
                ctx.output_type,
                ctx.prefix + inner_code + ctx.suffix,
                ctx.transform_verifier_block(item.verifier, pre, post),
                next_path,
                ctx.prefix + inner_prefix,
                item.depth + 1,
                next_rec_depth
            };
        };

        cb.should_abort = [] { return HasFailure(); };

        int start_depth = initial_item.depth;
        int start_rec_depth = initial_item.recursion_depth;

        ContextTreeWalker<ProcessingItem>::walk(std::move(initial_item), start_depth, start_rec_depth, cb);
    }


    ConstructedRecoveryProgram ParserTestBase::BuildRecoveryProgram(std::string inner_code,
                                                                    ProgramSpec inner_spec,
                                                                    const std::string& inner_prefix,
                                                                    size_t seed)
    {
        RecoveryBlock pre = RecoverySentinel::generate_top_level_sentinel(seed);
        RecoveryBlock post = RecoverySentinel::generate_top_level_sentinel(seed + 1);

        while (!inner_code.empty() && (inner_code.back() == '\n' || inner_code.back() == '\r'))
        {
            inner_code.pop_back();
        }

        std::string full_code = pre.source + "\n\n" + inner_code + "\n\n" + post.source + "\n";

        std::string prefix_for_shifting = pre.source + "\n\n" + inner_prefix;

        ProgramSpec full_spec;
        if (pre.add_to_spec) pre.add_to_spec(full_spec);
        full_spec = MergeSpecs(std::move(full_spec), std::move(inner_spec));
        if (post.add_to_spec) post.add_to_spec(full_spec);

        return {std::move(full_code), std::move(full_spec), std::move(prefix_for_shifting)};
    }

    void ParserTestBase::RunRecoveryScenario(ProcessingItem&& item,
                                             const std::vector<ParserExpectedError>& errors,
                                             size_t seed)
    {
        ProgramSpec item_spec;
        std::visit([&](auto&& ver) { SpecAdder::add(item_spec, ver); }, item.verifier);

        auto prog = BuildRecoveryProgram(std::move(item.code),
                                         std::move(item_spec),
                                         item.cumulative_prefix,
                                         seed);

        auto shifted = ErrorShifter::shift_errors(prog.prefix_for_shifting, errors);

        SCOPED_TRACE("Recovery Path: " + item.path_name);
        ExpectParseErrors(prog.full_code, shifted, prog.full_spec);
    }

    void ParserTestBase::ExpectValidParse(const std::string& code, const ProgramSpec& spec)
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

    void ParserTestBase::ExpectParseErrors(const std::string& code,
                                           const std::vector<ParserExpectedError>& expected_errors,
                                           const std::optional<ProgramSpec>& spec)
    {
        SCOPED_TRACE(format_source_with_lines(code));
        CompilerContext context;
        context.settings.fail_fast = false;
        std::shared_ptr<Program> ast = run_parser(code, context);

        const auto& actual_errors = context.diagnostics.get_errors();
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

    void ParserTestBase::ExpectParseErrorsWithRecovery(const std::string& code,
                                                       const std::vector<ParserExpectedError>& expected_errors,
                                                       ProgramSpec broken_part_spec)
    {
        auto* test_info = testing::UnitTest::GetInstance()->current_test_info();
        size_t base_seed = std::hash<std::string>{}(test_info ? test_info->name() : "fallback");

        auto prog = BuildRecoveryProgram(code, std::move(broken_part_spec), "", base_seed);

        auto shifted = ErrorShifter::shift_errors(prog.prefix_for_shifting, expected_errors);

        ExpectParseErrors(prog.full_code, shifted, prog.full_spec);
    }
}
