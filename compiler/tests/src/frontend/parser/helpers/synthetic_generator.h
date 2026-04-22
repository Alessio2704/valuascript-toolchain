#pragma once

#include <random>
#include <string>
#include <vector>
#include <functional>
#include <initializer_list>
#include <array>
#include <numeric>
#include <utility>

#include "construct_registry.h"
#include "context_registry.h"
#include "synthetic_generator_config.h"

namespace valuascript::compiler::test
{
    class SyntheticGenerator
    {
    public:
        using SpecAdderFn = std::function<void(ProgramSpec&)>;
        using PieceGenerator = std::function<std::pair<std::string, SpecAdderFn>()>;

        explicit SyntheticGenerator(size_t seed, SyntheticGeneratorConfig config = {});

        std::pair<std::string, ProgramSpec> generate_program(int piece_count);

        std::pair<std::string, std::vector<ModifierSpec>> synth_modifiers(int count);
        std::pair<std::string, TypeVerifier> synth_type(int depth = 0);
        std::pair<std::string, ExprVerifier> synth_expression(int depth = 0, int max_depth = 3);
        std::pair<std::string, StmtVerifier> synth_statement();

        std::pair<std::string, ParamSpec> synth_parameter(bool allow_default);
        std::pair<std::string, FieldSpec> synth_struct_field();
        std::pair<std::string, EnumCaseSpec> synth_enum_case();
        std::pair<std::string, std::vector<AssignmentTargetSpec>> synth_assignment_targets(int count);

        std::pair<std::string, StmtVerifier> logic_synth_statement();
        std::pair<std::string, FuncVerifier> logic_synth_function();
        std::pair<std::string, StructVerifier> logic_synth_struct();
        std::pair<std::string, EnumVerifier> logic_synth_enum();
        std::pair<std::string, AliasVerifier> logic_synth_type_alias();
        std::pair<std::string, ImportVerifier> logic_synth_import();
        std::pair<std::string, DirectiveVerifier> logic_synth_directive();
        std::pair<std::string, StmtVerifier> harvest_statement();

        std::string next_id();
        int rand_range(int min, int max);
        int rand_range(const std::pair<int, int>& range);
        bool roll_prob(double p);
        TopLevelConstruct roll_top_level_construct();

        template <typename EnumType>
        EnumType roll_weighted(const std::initializer_list<double> weights, EnumType default_val)
        {
            double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
            if (sum <= 0.0) return default_val;

            return static_cast<EnumType>(std::discrete_distribution<int>(weights.begin(), weights.end())(rng_));
        }

        template <typename T>
        const T& pick_random(const std::vector<T>& items)
        {
            if (items.empty())
            {
                static const T empty_val{};
                return empty_val;
            }
            return items[rand_range(0, static_cast<int>(items.size()) - 1)];
        }

        template <typename TargetVerifier, typename RegistryPoolVerifier>
        void register_hybrid_generator(TopLevelConstruct construct,
                                       InjectableType type,
                                       double registry_chance,
                                       const std::vector<RegistryEntry<RegistryPoolVerifier>>& pool,
                                       const std::function<std::pair<std::string, TargetVerifier>()>& synth_fn)
        {
            generators_[static_cast<int>(construct)] = [this, type, registry_chance, &pool, synth_fn
                ]() -> std::pair<std::string, SpecAdderFn>
                {
                    auto contexts = ContextRegistry::get_all_for(type);
                    auto ctx = pick_random(contexts);
                    std::string code;
                    UniversalVerifier v;

                    if (!pool.empty() && roll_prob(registry_chance))
                    {
                        auto& itm = pick_random(pool);
                        code = itm.code;
                        v = UniversalVerifier(TargetVerifier(itm.verifier));
                    }
                    else
                    {
                        auto [s_code, s_v] = synth_fn();
                        code = s_code;
                        v = UniversalVerifier(s_v);
                    }
                    return {ctx.prefix + code + ctx.suffix, [ctx, v](ProgramSpec& s) { ctx.wrap_in_spec(s, v); }};
                };
        }

        template <typename VerifierType, typename RegistryPoolType>
        void register_pyramid_member(TopLevelConstruct construct,
                                     InjectableType type,
                                     double registry_chance,
                                     const std::vector<RegistryEntry<RegistryPoolType>>& pool,
                                     const std::function<std::pair<std::string, VerifierType>()>& synth_fn)
        {
            generators_[static_cast<int>(construct)] = [=, &pool, this]()
            {
                auto contexts = ContextRegistry::get_all_for(type);
                auto ctx = pick_random(contexts);

                std::string code;
                VerifierType verifier;

                if (!pool.empty() && roll_prob(registry_chance))
                {
                    auto& item = pick_random(pool);
                    code = item.code;
                    verifier = item.verifier;
                }
                else
                {
                    auto [s_code, s_v] = synth_fn();
                    code = s_code;
                    verifier = s_v;
                }

                return apply_nesting_pyramid(ctx, code, UniversalVerifier(verifier));
            };
        }

    private:
        std::mt19937 rng_;
        size_t unique_id_ = 0;
        SyntheticGeneratorConfig config_;
        std::array<PieceGenerator, static_cast<int>(TopLevelConstruct::Count)> generators_;

        void setup_generators();
        std::pair<std::string, SpecAdderFn> apply_nesting_pyramid(const Context& inner_ctx,
                                                                  const std::string& atom_code,
                                                                  const UniversalVerifier& atom_verifier);
    };
}
