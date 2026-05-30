#pragma once

#include <random>
#include <string>
#include <vector>
#include <functional>
#include <initializer_list>
#include <array>
#include <numeric>
#include <utility>
#include <optional>

#include "frontend/parser/helpers/construct_registry.h"
#include "frontend/parser/helpers/context_registry.h"
#include "frontend/parser/helpers/spec_adder.h"
#include "../expansion_and_sentinels/context_tree_walker.h"
#include "synthetic_generator_config.h"
#include "synthetic_generator_stats.h"

namespace valuascript::compiler::test
{
    class SyntheticGenerator;

    template <typename T>
    using GenRule = std::function<std::pair<std::string, T>(SyntheticGenerator&, int)>;

    template <typename T>
    GenRule<std::vector<T>> Repeat(
        GenRule<T> rule,
        const std::function<std::pair<int, int>(const SyntheticGeneratorConfig&)>& range_fn,
        const std::string& separator = ""
    );

    template <typename T>
    GenRule<std::optional<T>> Opt(
        GenRule<T> rule,
        const std::function<double(const SyntheticGeneratorConfig&)>& prob_fn
    );

    template <typename VerifierType>
    GenRule<VerifierType> WithRegistryFallback(
        GenRule<VerifierType> generated_rule,
        const std::function<double(const SyntheticGeneratorConfig&)>& prob_fn,
        const std::vector<RegistryEntry<VerifierType>>* pool
    );

    template <typename VerifierT>
    struct GrammarRule
    {
        std::string name;
        GenRule<VerifierT> rule;
        std::string test_prefix;
        std::string test_suffix;
        std::function<void(Program*, const VerifierT&)> validate;
    };

    class SyntheticGenerator
    {
    public:
        using SpecAdderFn = std::function<void(ProgramSpec&)>;
        using PieceGenerator = std::function<std::pair<std::string, SpecAdderFn>()>;

        explicit SyntheticGenerator(size_t seed,
                                    SyntheticGeneratorConfig config =
                                        SyntheticGeneratorConfig::default_fuzzing_config());

        std::pair<std::string, ProgramSpec> generate_program(int piece_count);

        std::pair<std::string, std::vector<ModifierSpec>> generate_raw_modifiers(int depth = 0)
        {
            return rule_modifiers(*this, depth);
        }

        std::pair<std::string, std::vector<ModifierSpec>> generate_raw_standalone_modifiers(int depth = 0)
        {
            return rule_standalone_modifiers(*this, depth);
        }

        std::pair<std::string, TypeVerifier> generate_raw_type(int depth = 0) { return rule_type(*this, depth); }

        std::pair<std::string, ExprVerifier> generate_raw_expression(int depth = 0)
        {
            return rule_expression(*this, depth);
        }

        std::pair<std::string, StmtVerifier> generate_raw_statement(int depth = 0)
        {
            return rule_statement(*this, depth);
        }

        std::pair<std::string, ParamSpec> generate_raw_parameter(int depth = 0) { return rule_parameter(*this, depth); }

        std::pair<std::string, FieldSpec> generate_raw_struct_field(int depth = 0)
        {
            return rule_struct_field(*this, depth);
        }

        std::pair<std::string, EnumCaseSpec> generate_raw_enum_case(int depth = 0)
        {
            return rule_enum_case(*this, depth);
        }

        std::pair<std::string, std::vector<AssignmentTargetSpec>> generate_raw_assignment_targets(int depth = 0)
        {
            return rule_assignment_targets(*this, depth);
        }

        std::pair<std::string, ReturnVerifier> generate_raw_return(int depth = 0) { return rule_return(*this, depth); }

        std::pair<std::string, FuncVerifier> generate_raw_function(int depth = 0)
        {
            return rule_function(*this, depth);
        }

        std::pair<std::string, StructVerifier> generate_raw_struct(int depth = 0) { return rule_struct(*this, depth); }
        std::pair<std::string, EnumVerifier> generate_raw_enum(int depth = 0) { return rule_enum(*this, depth); }

        std::pair<std::string, AliasVerifier> generate_raw_alias(int depth = 0)
        {
            return rule_type_alias(*this, depth);
        }

        std::pair<std::string, ImportVerifier> generate_raw_import(int depth = 0) { return rule_import(*this, depth); }

        std::pair<std::string, DirectiveVerifier> generate_raw_directive(int depth = 0)
        {
            return rule_directive(*this, depth);
        }

        std::string next_id();
        int rand_range(int min, int max);
        int rand_range(const std::pair<int, int>& range);
        bool roll_prob(double p);
        TopLevelConstruct roll_top_level_construct();

        [[nodiscard]] size_t get_unique_id() const { return unique_id_; }
        [[nodiscard]] const SyntheticGeneratorConfig& get_config() const { return config_; }
        SyntheticGenerationStats& stats() { return stats_; }
        [[nodiscard]] const SyntheticGenerationStats& get_stats() const { return stats_; }

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
            return items[static_cast<size_t>(rand_range(0, static_cast<int>(items.size()) - 1))];
        }

        template <typename Func>
        void for_each_rule(Func&& f)
        {
            f(GrammarRule<std::vector<ModifierSpec>>{
                "Modifiers", rule_modifiers, "let ", "x = 1\n",
                [](const Program* p, const std::vector<ModifierSpec>& v)
                {
                    if (p && !p->execution_steps.empty())
                    {
                        if (auto a = dynamic_cast<Assignment*>(p->execution_steps[0].get()))
                        {
                            ExpectModifiers(a->targets[0].modifiers, v);
                        }
                    }
                }
            });

            f(GrammarRule<std::vector<ModifierSpec>>{
                "StandaloneModifiers", rule_standalone_modifiers, "func _w() -> any {\n  ", "return 1\n}\n",
                [](const Program* p, const std::vector<ModifierSpec>& v)
                {
                    if (p && !p->function_definitions.empty())
                    {
                        auto func = p->function_definitions[0].get();
                        if (!func->body.empty())
                        {
                            if (auto r = dynamic_cast<ReturnStatement*>(func->body[0].get()))
                            {
                                ExpectModifiers(r->modifiers, v);
                            }
                        }
                    }
                }
            });

            f(GrammarRule<TypeVerifier>{
                "Type", rule_type, "let x: ", " = 1\n",
                [](const Program* p, const TypeVerifier& v)
                {
                    if (p && !p->execution_steps.empty())
                    {
                        if (auto a = dynamic_cast<Assignment*>(p->execution_steps[0].get()))
                        {
                            if (v) v(a->targets[0].type.get());
                        }
                    }
                }
            });

            f(GrammarRule<ExprVerifier>{
                "Expression", rule_expression, "let _x = ", "\n",
                [](const Program* p, const ExprVerifier& v)
                {
                    if (p && !p->execution_steps.empty())
                    {
                        if (auto a = dynamic_cast<Assignment*>(p->execution_steps[0].get()))
                        {
                            if (v) v(a->value.get());
                        }
                    }
                }
            });

            f(GrammarRule<StmtVerifier>{
                "Statement", rule_statement, "", "\n",
                [](const Program* p, const StmtVerifier& v)
                {
                    if (p && !p->execution_steps.empty())
                    {
                        if (v) v(p->execution_steps[0].get());
                    }
                }
            });

            f(GrammarRule<ParamSpec>{
                "Parameter", rule_parameter, "func _f(", ") -> void {}\n",
                [](const Program* p, const ParamSpec& v)
                {
                    if (p && !p->function_definitions.empty())
                    {
                        auto func = p->function_definitions[0].get();
                        if (!func->parameters.empty())
                        {
                            EXPECT_EQ(func->parameters[0].name, v.name);
                            ExpectModifiers(func->parameters[0].modifiers, v.modifiers);
                            if (v.type_v) v.type_v(func->parameters[0].type.get());
                            if (v.default_v) v.default_v(func->parameters[0].default_value.get());
                        }
                    }
                }
            });

            f(GrammarRule<FieldSpec>{
                "StructField", rule_struct_field, "struct _S { ", " }\n",
                [](const Program* p, const FieldSpec& v)
                {
                    if (p && !p->struct_definitions.empty())
                    {
                        auto s = p->struct_definitions[0].get();
                        if (!s->fields.empty())
                        {
                            EXPECT_EQ(s->fields[0].name, v.name);
                            ExpectModifiers(s->fields[0].modifiers, v.modifiers);
                            if (v.type_v) v.type_v(s->fields[0].type.get());
                        }
                    }
                }
            });

            f(GrammarRule<EnumCaseSpec>{
                "EnumCase", rule_enum_case, "enum _E: int { ", " }\n",
                [](const Program* p, const EnumCaseSpec& v)
                {
                    if (p && !p->enum_definitions.empty())
                    {
                        auto e = p->enum_definitions[0].get();
                        if (!e->cases.empty())
                        {
                            EXPECT_EQ(e->cases[0].name, v.name);
                            ExpectModifiers(e->cases[0].modifiers, v.modifiers);
                            if (v.value_v) v.value_v(e->cases[0].value.get());
                        }
                    }
                }
            });

            f(GrammarRule<std::vector<AssignmentTargetSpec>>{
                "AssignmentTargets", rule_assignment_targets, "let ", " = 1\n",
                [](const Program* p, const std::vector<AssignmentTargetSpec>& v)
                {
                    if (p && !p->execution_steps.empty())
                    {
                        if (auto a = dynamic_cast<Assignment*>(p->execution_steps[0].get()))
                        {
                            EXPECT_EQ(a->targets.size(), v.size());
                            for (size_t i = 0; i < v.size() && i < a->targets.size(); ++i)
                            {
                                EXPECT_EQ(a->targets[i].name, v[i].name);
                                ExpectModifiers(a->targets[i].modifiers, v[i].modifiers);
                                if (v[i].type_v) v[i].type_v(a->targets[i].type.get());
                            }
                        }
                    }
                }
            });

            f(GrammarRule<ReturnVerifier>{
                "Return", rule_return, "func _w() -> any {\n  ", "}\n",
                [](const Program* p, const ReturnVerifier& v)
                {
                    if (p && !p->function_definitions.empty())
                    {
                        auto func = p->function_definitions[0].get();
                        if (!func->body.empty())
                        {
                            if (auto r = dynamic_cast<ReturnStatement*>(func->body[0].get()))
                            {
                                if (v) v(r);
                            }
                        }
                    }
                }
            });

            f(GrammarRule<FuncVerifier>{
                "Function", rule_function, "", "",
                [](const Program* p, const FuncVerifier& v)
                {
                    if (p && !p->function_definitions.empty())
                    {
                        if (v) v(p->function_definitions[0].get());
                    }
                }
            });

            f(GrammarRule<StructVerifier>{
                "Struct", rule_struct, "", "",
                [](const Program* p, const StructVerifier& v)
                {
                    if (p && !p->struct_definitions.empty())
                    {
                        if (v) v(p->struct_definitions[0].get());
                    }
                }
            });

            f(GrammarRule<EnumVerifier>{
                "Enum", rule_enum, "", "",
                [](const Program* p, const EnumVerifier& v)
                {
                    if (p && !p->enum_definitions.empty())
                    {
                        if (v) v(p->enum_definitions[0].get());
                    }
                }
            });

            f(GrammarRule<AliasVerifier>{
                "TypeAlias", rule_type_alias, "", "",
                [](const Program* p, const AliasVerifier& v)
                {
                    if (p && !p->type_aliases.empty())
                    {
                        if (v) v(p->type_aliases[0].get());
                    }
                }
            });

            f(GrammarRule<ImportVerifier>{
                "Import", rule_import, "", "",
                [](const Program* p, const ImportVerifier& v)
                {
                    if (p && !p->import_statements.empty())
                    {
                        if (v) v(p->import_statements[0].get());
                    }
                }
            });

            f(GrammarRule<DirectiveVerifier>{
                "Directive", rule_directive, "", "",
                [](const Program* p, const DirectiveVerifier& v)
                {
                    if (p && !p->directives.empty())
                    {
                        if (v) v(p->directives[0].get());
                    }
                }
            });
        }

    private:
        std::mt19937 rng_;
        size_t unique_id_ = 0;
        SyntheticGeneratorConfig config_;
        SyntheticGenerationStats stats_;
        std::array<PieceGenerator, static_cast<int>(TopLevelConstruct::Count)> generators_;

        GenRule<std::vector<ModifierSpec>> rule_modifiers;
        GenRule<std::vector<ModifierSpec>> rule_standalone_modifiers;
        GenRule<TypeVerifier> rule_type;
        GenRule<ExprVerifier> rule_expression;
        GenRule<StmtVerifier> rule_statement;
        GenRule<ParamSpec> rule_parameter;
        GenRule<FieldSpec> rule_struct_field;
        GenRule<EnumCaseSpec> rule_enum_case;
        GenRule<std::vector<AssignmentTargetSpec>> rule_assignment_targets;
        GenRule<ReturnVerifier> rule_return;
        GenRule<FuncVerifier> rule_function;
        GenRule<StructVerifier> rule_struct;
        GenRule<EnumVerifier> rule_enum;
        GenRule<AliasVerifier> rule_type_alias;
        GenRule<ImportVerifier> rule_import;
        GenRule<DirectiveVerifier> rule_directive;

        void build_grammar();
        void setup_generators();

        std::pair<std::string, SpecAdderFn> walk_to_top_level(InjectableType start_type,
                                                              const std::string& atom_code,
                                                              const UniversalVerifier& atom_verifier);

        template <typename VerifierType>
        void register_top_level(TopLevelConstruct construct,
                                const std::function<std::pair<std::string, VerifierType>()>& rule_fn)
        {
            generators_[static_cast<size_t>(construct)] = [=]() -> std::pair<std::string, SpecAdderFn>
            {
                auto [code, verifier] = rule_fn();
                return {
                    code, [verifier](ProgramSpec& s)
                    {
                        std::visit([&](auto&& ver) { SpecAdder::add(s, ver); }, UniversalVerifier(verifier));
                    }
                };
            };
        }

        template <typename VerifierType>
        void register_pyramid_member(TopLevelConstruct construct,
                                     InjectableType type,
                                     const std::function<std::pair<std::string, VerifierType>()>& rule_fn)
        {
            generators_[static_cast<size_t>(construct)] = [=, this]()
            {
                auto [code, verifier] = rule_fn();
                return walk_to_top_level(type, code, UniversalVerifier(verifier));
            };
        }
    };

    template <typename T>
    GenRule<std::vector<T>> Repeat(
        GenRule<T> rule,
        const std::function<std::pair<int, int>(const SyntheticGeneratorConfig&)>& range_fn,
        const std::string& separator
    )
    {
        return [rule, range_fn, separator](SyntheticGenerator& env, int depth)
        {
            int count = env.rand_range(range_fn(env.get_config()));
            std::string code;
            std::vector<T> verifiers;
            verifiers.reserve(static_cast<size_t>(count));
            for (int i = 0; i < count; ++i)
            {
                auto res = rule(env, depth);
                code += res.first;
                verifiers.push_back(res.second);
                if (i < count - 1) code += separator;
            }
            return std::make_pair(code, verifiers);
        };
    }

    template <typename T>
    GenRule<std::optional<T>> Opt(
        GenRule<T> rule,
        const std::function<double(const SyntheticGeneratorConfig&)>& prob_fn
    )
    {
        return [rule, prob_fn](SyntheticGenerator& env, int depth)
        {
            if (env.roll_prob(prob_fn(env.get_config())))
            {
                auto res = rule(env, depth);
                return std::make_pair(res.first, std::make_optional(res.second));
            }
            return std::make_pair(std::string(""), std::optional<T>(std::nullopt));
        };
    }

    template <typename VerifierType>
    GenRule<VerifierType> WithRegistryFallback(
        GenRule<VerifierType> generated_rule,
        const std::function<double(const SyntheticGeneratorConfig&)>& prob_fn,
        const std::vector<RegistryEntry<VerifierType>>* pool
    )
    {
        return [generated_rule, prob_fn, pool](SyntheticGenerator& env, int depth)
        {
            env.stats().total_nodes_generated++;
            if (pool && !pool->empty() && env.roll_prob(prob_fn(env.get_config())))
            {
                env.stats().registry_fallbacks++;
                const auto& itm = env.pick_random(*pool);
                return std::make_pair(itm.code, VerifierType(itm.verifier));
            }
            env.stats().synthesized_from_scratch++;
            return generated_rule(env, depth);
        };
    }
}
