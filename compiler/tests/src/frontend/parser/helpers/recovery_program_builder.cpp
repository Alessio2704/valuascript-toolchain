#include "recovery_program_builder.h"
#include "recovery_sentinel.h"
#include "error_shifter.h"
#include "parser_runner.h"
#include "spec_adder.h"

namespace valuascript::compiler::test
{
    std::vector<ConstructedRecoveryProgram> RecoveryProgramBuilder::BuildRecoveryPrograms(
        std::string inner_code,
        ProgramSpec inner_spec,
        const std::string& inner_prefix,
        size_t seed,
        const std::vector<SentinelKind>& excluded_sentinels,
        const std::vector<SentinelKind>& accepted_sentinels,
        const std::string& path_name)
    {
        std::vector<SentinelKind> effective_accepted;
        if (!accepted_sentinels.empty())
        {
            effective_accepted.reserve(accepted_sentinels.size());
            for (auto kind : accepted_sentinels)
            {
                if (std::find(excluded_sentinels.begin(), excluded_sentinels.end(), kind) == excluded_sentinels.end())
                {
                    effective_accepted.push_back(kind);
                }
            }
        }

        auto build_single = [&](size_t s, const std::vector<SentinelKind>& acc,
                                const std::string& path_suffix) -> ConstructedRecoveryProgram
        {
            RecoveryBlock pre = RecoverySentinel::generate_block_sentinel(
                s, BlockContext::TopLevel, excluded_sentinels, {});
            RecoveryBlock post = RecoverySentinel::generate_block_sentinel(
                s + 1, BlockContext::TopLevel, excluded_sentinels, acc);

            std::string code = inner_code;
            while (!code.empty() && (code.back() == '\n' || code.back() == '\r'))
            {
                code.pop_back();
            }

            std::string full_code = pre.source + "\n\n" + code + "\n\n" + post.source + "\n";
            std::string prefix_for_shifting = pre.source + "\n\n" + inner_prefix;

            ProgramSpec full_spec;
            if (pre.add_to_spec) pre.add_to_spec(full_spec);
            full_spec = MergeSpecs(std::move(full_spec), inner_spec);
            if (post.add_to_spec) post.add_to_spec(full_spec);

            std::string full_path = path_name.empty() ? "" : (path_name + path_suffix);
            return ConstructedRecoveryProgram{
                .full_code = std::move(full_code),
                .full_spec = std::move(full_spec),
                .prefix_for_shifting = std::move(prefix_for_shifting),
                .path_name = std::move(full_path)
            };
        };

        if (effective_accepted.size() > 1)
        {
            std::vector<ConstructedRecoveryProgram> results;
            results.reserve(effective_accepted.size());
            for (size_t i = 0; i < effective_accepted.size(); ++i)
            {
                std::string tag = " [" + to_string(effective_accepted[i]) + "]";
                results.push_back(build_single(seed + (i * 2), {effective_accepted[i]}, tag));
            }
            return results;
        }

        return {build_single(seed, effective_accepted, "")};
    }

    std::vector<ConstructedRecoveryProgram> RecoveryProgramBuilder::BuildRecoveryPrograms(
        const ProcessingItem& item,
        size_t seed,
        std::optional<ProgramSpec> inner_spec_override)
    {
        ProgramSpec spec;
        if (inner_spec_override.has_value())
        {
            spec = std::move(inner_spec_override.value());
        }
        else
        {
            std::visit([&](auto&& ver) { SpecAdder::add(spec, ver); }, item.verifier);
        }

        return BuildRecoveryPrograms(
            item.code,
            std::move(spec),
            item.cumulative_prefix,
            seed,
            item.excluded_sentinels,
            item.accepted_sentinels,
            item.path_name
        );
    }

    void RecoveryProgramBuilder::RunRecoveryScenario(ProcessingItem&& item,
                                             const std::vector<ParserExpectedError>& errors,
                                             size_t seed)
    {
        ForEachRecoveryProgram(item, seed, [&](const ConstructedRecoveryProgram& prog)
        {
            const auto& errors_to_use = item.custom_errors.has_value() ? item.custom_errors.value() : errors;
            auto shifted = ErrorShifter::shift_errors(prog.prefix_for_shifting, errors_to_use);
            ParserRunner::ExpectParseErrors(prog.full_code, shifted, prog.full_spec);
        });
    }
}
