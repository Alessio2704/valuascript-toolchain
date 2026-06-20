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
                config_.weights.top_level_constructs.extension_def,
                config_.weights.top_level_constructs.struct_def,
                config_.weights.top_level_constructs.enum_def,
                config_.weights.top_level_constructs.type_alias,
                config_.weights.top_level_constructs.import_stmt,
                config_.weights.top_level_constructs.directive
            }, -1);

        if (result == -1) return TopLevelConstruct::None;
        return static_cast<TopLevelConstruct>(result + 1);
    }

    std::pair<std::string, SyntheticGenerator::SpecAdderFn> SyntheticGenerator::walk_to_top_level(
        InjectableType start_type,
        const std::string& atom_code,
        const UniversalVerifier& atom_verifier)
    {
        struct FuzzState
        {
            InjectableType type;
            std::string code;
            UniversalVerifier verifier;
        };

        std::pair<std::string, SpecAdderFn> result = {
            "", [](ProgramSpec&)
            {
            }
        };

        while (result.first.empty())
        {
            FuzzState current{start_type, atom_code, atom_verifier};

            ContextTreeWalker<FuzzState>::Callbacks cb;
            cb.get_type = [](const FuzzState& s) { return s.type; };

            cb.on_terminal = [&](const FuzzState& s)
            {
                result = {
                    s.code,
                    [v = s.verifier](ProgramSpec& spec)
                    {
                        std::visit([&](auto&& ver) { SpecAdder::add(spec, ver); }, v);
                    }
                };
            };

            cb.on_promotion = [&](const FuzzState& s)
            {
                FuzzState promoted{InjectableType::TopLevel, s.code, s.verifier};
                cb.on_terminal(promoted);
            };

            cb.on_normal_branch = [](const FuzzState& s, const Context& ctx, int)
            {
                return FuzzState{
                    ctx.output_type,
                    ctx.prefix + s.code + ctx.suffix,
                    ctx.transform_verifier(s.verifier)
                };
            };

            cb.on_block_branch = [](const FuzzState& s, const Context& ctx, int)
            {
                std::vector<RecoveryBlock> empty_blocks;
                return FuzzState{
                    ctx.output_type,
                    ctx.prefix + s.code + ctx.suffix,
                    ctx.transform_verifier_block(s.verifier, empty_blocks, empty_blocks)
                };
            };

            cb.strategy = [this](const std::vector<NextStep>& steps, int, int) -> std::vector<NextStep>
            {
                if (steps.empty()) return {};
                auto idx = static_cast<size_t>(this->rand_range(0, static_cast<int>(steps.size()) - 1));
                return {steps[idx]};
            };

            ContextTreeWalker<FuzzState>::walk(
                std::move(current), 0, 0, cb,
                ExpansionPolicy{
                    config_.sizes.expansion_policy_max_steps_retries.first,
                    config_.sizes.expansion_policy_max_steps_retries.second
                }
            );
        }

        return result;
    }

    std::pair<std::string, ProgramSpec> SyntheticGenerator::generate_program(int piece_count)
    {
        std::vector<std::pair<std::string, SpecAdderFn>> pieces;
        pieces.reserve(static_cast<size_t>(piece_count));

        for (int i = 0; i < piece_count; ++i)
        {
            TopLevelConstruct choice = roll_top_level_construct();
            if (choice == TopLevelConstruct::None) continue;

            switch (choice)
            {
            case TopLevelConstruct::Expression: stats_.pieces_rolled.expressions++;
                break;
            case TopLevelConstruct::TypeAnnotation: stats_.pieces_rolled.type_annotations++;
                break;
            case TopLevelConstruct::Statement: stats_.pieces_rolled.statements++;
                break;
            case TopLevelConstruct::ReturnStmt: stats_.pieces_rolled.return_stmts++;
                break;
            case TopLevelConstruct::Modifier: stats_.pieces_rolled.modifiers++;
                break;
            case TopLevelConstruct::FunctionDef: stats_.pieces_rolled.functions++;
                break;
            case TopLevelConstruct::ExtensionDef: stats_.pieces_rolled.extensions++;
                break;
            case TopLevelConstruct::StructDef: stats_.pieces_rolled.structs++;
                break;
            case TopLevelConstruct::EnumDef: stats_.pieces_rolled.enums++;
                break;
            case TopLevelConstruct::TypeAlias: stats_.pieces_rolled.type_aliases++;
                break;
            case TopLevelConstruct::ImportStmt: stats_.pieces_rolled.imports++;
                break;
            case TopLevelConstruct::Directive: stats_.pieces_rolled.directives++;
                break;
            default: break;
            }

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
