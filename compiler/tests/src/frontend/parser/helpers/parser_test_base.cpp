#include "parser_test_base.h"
#include "frontend/parser/expansion_and_sentinels/expansion_calculator.h"
#include "spec_adder.h"
#include "error_shifter.h"

namespace valuascript::compiler::test
{
    std::string ParserTestBase::format_source_with_lines(const std::string& code)
    {
        return ParserRunner::format_source_with_lines(code);
    }

    std::shared_ptr<Program> ParserTestBase::run_parser(const std::string& code, CompilerContext& context)
    {
        return ParserRunner::run_parser(code, context);
    }

    std::vector<ProcessingItem> ParserTestBase::apply_context_augmentations(
        InjectableType type,
        const std::string& snippet,
        const UniversalVerifier& verifier,
        const std::string& group_name,
        const std::vector<std::string_view>& skip_contexts,
        const std::vector<ContextOverrideAny>& context_overrides,
        const std::vector<SentinelKind>& excluded_sentinels,
        const std::vector<SentinelKind>& accepted_sentinels
    )
    {
        return StreamExpander::apply_context_augmentations(
            type, snippet, verifier, group_name, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
    }

    void ParserTestBase::expand_to_top_level_stream(std::vector<ProcessingItem> items,
                                                    const ExpansionCallback& callback,
                                                    bool inject_sentinels,
                                                    std::optional<ExpansionPolicy> policy_override)
    {
        StreamExpander::expand_to_top_level_stream(std::move(items), callback, inject_sentinels, policy_override);
    }

    void ParserTestBase::ExpectValidUnified(InjectableType type, std::vector<ProcessingItem> items,
                                            const std::string& group_name)
    {
        size_t actual_expansions = 0;

        expand_to_top_level_stream(std::move(items), [&](ProcessingItem&& item)
        {
            actual_expansions++;
            ProgramSpec spec;
            std::visit([&](auto&& ver) { SpecAdder::add(spec, ver); }, item.verifier);
            SCOPED_TRACE("Context: " + item.path_name);
            ExpectValidParse(item.code, spec);
        }, false);

        if (!HasFailure() && !items.empty())
        {
            size_t expected_expansions = ExpansionCalculator::compute_expected_expansions(type, items[0].skip_contexts);
            EXPECT_EQ(actual_expansions, expected_expansions * items.size()) << "Expansion count mismatch for " <<
                group_name << " (Valid Parse).";
        }
    }

    void ParserTestBase::ExpectParseErrorsUnified(InjectableType type, std::vector<ProcessingItem> items,
                                                  const std::vector<ParserExpectedError>& errors,
                                                  const std::string& group_name)
    {
        auto* test_info = testing::UnitTest::GetInstance()->current_test_info();
        size_t base_seed = DeterministicSampler::make_seed(test_info ? test_info->name() : "fallback");

        size_t actual_expansions = 0;
        size_t scenario_index = 0;

        expand_to_top_level_stream(std::move(items), [&](ProcessingItem&& item)
        {
            actual_expansions++;
            RunRecoveryScenario(std::move(item), errors, base_seed + (scenario_index++ * 2));
        }, true);

        if (!HasFailure() && !items.empty())
        {
            size_t expected_expansions = ExpansionCalculator::compute_expected_expansions(type, items[0].skip_contexts);
            EXPECT_EQ(actual_expansions, expected_expansions * items.size()) << "Expansion count mismatch for " <<
                group_name << " (Error Recovery).";
        }
    }

    std::vector<ConstructedRecoveryProgram> ParserTestBase::BuildRecoveryPrograms(
        std::string inner_code,
        ProgramSpec inner_spec,
        const std::string& inner_prefix,
        size_t seed,
        const std::vector<SentinelKind>& excluded_sentinels,
        const std::vector<SentinelKind>& accepted_sentinels,
        const std::string& path_name)
    {
        return RecoveryProgramBuilder::BuildRecoveryPrograms(
            std::move(inner_code), std::move(inner_spec), inner_prefix, seed, excluded_sentinels, accepted_sentinels, path_name);
    }

    std::vector<ConstructedRecoveryProgram> ParserTestBase::BuildRecoveryPrograms(
        const ProcessingItem& item,
        size_t seed,
        std::optional<ProgramSpec> inner_spec_override)
    {
        return RecoveryProgramBuilder::BuildRecoveryPrograms(item, seed, std::move(inner_spec_override));
    }

    void ParserTestBase::RunRecoveryScenario(ProcessingItem&& item,
                                             const std::vector<ParserExpectedError>& errors,
                                             size_t seed)
    {
        RecoveryProgramBuilder::RunRecoveryScenario(std::move(item), errors, seed);
    }

    void ParserTestBase::ExpectValidParse(const std::string& code, const ProgramSpec& spec)
    {
        ParserRunner::ExpectValidParse(code, spec);
    }

    void ParserTestBase::ExpectParseErrors(const std::string& code,
                                           const std::vector<ParserExpectedError>& expected_errors,
                                           const std::optional<ProgramSpec>& spec)
    {
        ParserRunner::ExpectParseErrors(code, expected_errors, spec);
    }

    void ParserTestBase::ExpectParseErrorsWithRecovery(const std::string& code,
                                                       const std::vector<ParserExpectedError>& expected_errors,
                                                       ProgramSpec broken_part_spec)
    {
        ParserRunner::ExpectParseErrorsWithRecovery(code, expected_errors, std::move(broken_part_spec));
    }
}
