#include "synthetic_generator.h"
#include "frontend/parser/helpers/recovery_sentinel.h"
#include <sstream>
#include <algorithm>

namespace valuascript::compiler::test
{
    SyntheticGenerator::SyntheticGenerator(size_t seed, SyntheticGeneratorConfig config)
        : rng_(static_cast<unsigned int>(seed)), config_(std::move(config))
    {
        build_grammar();
        setup_generators();
    }

    std::string SyntheticGenerator::next_id()
    {
        return "synth_id_" + std::to_string(unique_id_++);
    }

    int SyntheticGenerator::rand_range(int min, int max)
    {
        if (min >= max) return min;
        return std::uniform_int_distribution<int>(min, max)(rng_);
    }

    int SyntheticGenerator::rand_range(const std::pair<int, int>& range)
    {
        return rand_range(range.first, range.second);
    }

    bool SyntheticGenerator::roll_prob(double p)
    {
        if (p <= 0.0) return false;
        if (p >= 1.0) return true;
        return std::bernoulli_distribution(p)(rng_);
    }

    TopLevelConstruct SyntheticGenerator::roll_top_level_construct()
    {
        int result = roll_weighted<int>(
            {
                config_.weights.top_level_constructs.expression,
                config_.weights.top_level_constructs.type_annotation,
                config_.weights.top_level_constructs.statement,
                config_.weights.top_level_constructs.return_stmt,
                config_.weights.top_level_constructs.modifier,
                config_.weights.top_level_constructs.function_def,
                config_.weights.top_level_constructs.struct_def,
                config_.weights.top_level_constructs.enum_def,
                config_.weights.top_level_constructs.type_alias,
                config_.weights.top_level_constructs.import_stmt,
                config_.weights.top_level_constructs.directive
            }, -1);

        if (result == -1) return TopLevelConstruct::None;
        return static_cast<TopLevelConstruct>(result + 1);
    }

    std::pair<std::string, SyntheticGenerator::SpecAdderFn> SyntheticGenerator::apply_nesting_pyramid(
        const Context& inner_ctx,
        const std::string& atom_code,
        const UniversalVerifier& atom_verifier)
    {
        std::string snippet = inner_ctx.prefix + atom_code + inner_ctx.suffix;
        UniversalVerifier transformed_verifier = inner_ctx.transform_verifier(atom_verifier);

        bool should_wrap = inner_ctx.output_type == InjectableType::WeakStatement ||
            (inner_ctx.output_type == InjectableType::StrongStatement && roll_prob(0.5));

        if (!should_wrap)
        {
            return {
                snippet,
                [transformed_verifier](ProgramSpec& s)
                {
                    std::visit([&](auto&& v) { SpecAdder::add(s, v); }, transformed_verifier);
                }
            };
        }

        auto wrappers = ContextRegistry::get_block_contexts();
        auto wrapper_ctx = pick_random(wrappers);

        std::string wrapped_code = wrapper_ctx.prefix + snippet + wrapper_ctx.suffix;
        UniversalVerifier final_verifier;

        if (wrapper_ctx.is_block_context)
        {
            std::vector<RecoveryBlock> empty_blocks;
            final_verifier = wrapper_ctx.transform_verifier_block(transformed_verifier, empty_blocks, empty_blocks);
        }
        else
        {
            final_verifier = wrapper_ctx.transform_verifier(transformed_verifier);
        }

        return {
            wrapped_code, [final_verifier](ProgramSpec& s)
            {
                std::visit([&](auto&& v) { SpecAdder::add(s, v); }, final_verifier);
            }
        };
    }

    std::pair<std::string, ProgramSpec> SyntheticGenerator::generate_program(int piece_count)
    {
        std::vector<std::pair<std::string, SpecAdderFn>> pieces;
        pieces.reserve(static_cast<size_t>(piece_count));

        for (int i = 0; i < piece_count; ++i)
        {
            TopLevelConstruct choice = roll_top_level_construct();
            if (choice == TopLevelConstruct::None) continue;
            pieces.push_back(generators_[static_cast<size_t>(choice)]());
        }

        std::shuffle(pieces.begin(), pieces.end(), rng_);
        std::stringstream full_code;
        ProgramSpec full_spec;

        for (auto& [code, adder] : pieces)
        {
            if (code.empty()) continue;
            full_code << code << "\n";
            adder(full_spec);
        }

        return {full_code.str(), full_spec};
    }
}
