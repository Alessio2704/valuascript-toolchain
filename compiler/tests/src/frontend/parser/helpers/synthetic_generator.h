#pragma once

#include <random>
#include <sstream>
#include <algorithm>
#include <vector>
#include <functional>

#include "construct_registry.h"
#include "expression_contexts_provider.h"
#include "type_annotation_contexts_provider.h"
#include "assignment_contexts_provider.h"
#include "reassignment_contexts_provider.h"
#include "return_statement_contexts_provider.h"
#include "expression_statement_contexts_provider.h"
#include "import_contexts_provider.h"
#include "directive_contexts_provider.h"
#include "function_definition_contexts_provider.h"
#include "struct_definition_contexts_provider.h"
#include "enum_definition_contexts_provider.h"
#include "type_alias_contexts_provider.h"
#include "modifier_contexts_provider.h"

namespace valuascript::compiler::test
{
    class SyntheticGenerator
    {
    private:
        std::mt19937 rng_;
        size_t unique_id_ = 0;

        using SpecAdder = std::function<void(ProgramSpec&)>;
        using PieceGenerator = std::function<std::pair<std::string, SpecAdder>()>;

        std::vector<PieceGenerator> generators_;

        std::string next_id() { return "synth_id_" + std::to_string(unique_id_++); }

        int rand_range(int min, int max)
        {
            if (min >= max) return min;
            return min + (rng_() % (max - min + 1));
        }

        template <typename T>
        const T& pick_random(const std::vector<T>& items)
        {
            return items[rand_range(0, items.size() - 1)];
        }

    public:
        explicit SyntheticGenerator(size_t seed) : rng_(seed)
        {
            setup_generators();
        }

        std::pair<std::string, std::vector<ModifierSpec>> synth_modifiers(int count)
        {
            const auto& pool = ConstructRegistry::modifiers();
            std::string code;
            std::vector<ModifierSpec> specs;

            if (count <= 0) return {"", {}};

            for (int i = 0; i < count; ++i)
            {
                if (!pool.empty() && rand_range(0, 1) == 0)
                {
                    const auto& item = pick_random(pool);
                    code += item.code + " ";
                    specs.insert(specs.end(), item.verifier.begin(), item.verifier.end());
                }
                else
                {
                    std::string mod_name = "mod_" + next_id();
                    std::vector<ArgSpec> args;
                    std::string arg_code;

                    int arg_count = rand_range(0, 3);
                    if (arg_count > 0)
                    {
                        arg_code += "(";
                        for (int j = 0; j < arg_count; ++j)
                        {
                            std::string arg_label = "arg_" + next_id();
                            auto [e_c, e_v] = synth_expression();

                            arg_code += arg_label + ": " + e_c;
                            args.push_back({arg_label, e_v});

                            if (j < arg_count - 1) arg_code += ", ";
                        }
                        arg_code += ")";
                    }

                    code += "@" + mod_name + arg_code + " ";
                    specs.push_back({mod_name, args});
                }
            }
            return {code, specs};
        }

        std::pair<std::string, TypeVerifier> synth_type(int depth = 0)
        {
            const auto& pool = ConstructRegistry::type_annotations();

            if (depth >= 2 || pool.empty() || rand_range(0, 2) == 0)
            {
                if (pool.empty()) return {"any", IsType("any")};
                const auto& item = pick_random(pool);
                return {item.code, item.verifier};
            }

            if (rand_range(0, 1) == 0)
            {
                auto [t1_c, t1_v] = synth_type(depth + 1);
                auto [t2_c, t2_v] = synth_type(depth + 1);
                return {"(" + t1_c + ", " + t2_c + ")", IsTupleType({t1_v, t2_v})};
            }

            auto [inner_c, inner_v] = synth_type(depth + 1);
            return {"List<" + inner_c + ">", IsType("List", {inner_v})};
        }

        std::pair<std::string, ExprVerifier> synth_expression(int depth = 0, int max_depth = 3)
        {
            const auto& pool = ConstructRegistry::expressions();

            if (depth >= max_depth || pool.empty())
            {
                if (pool.empty()) return {"0", IsNumber("0")};
                const auto& item = pick_random(pool);
                return {item.code, item.verifier};
            }

            auto [inner_c, inner_v] = synth_expression(depth + 1, max_depth);

            switch (rand_range(0, 5))
            {
            case 0:
                {
                    const auto& leaf = pick_random(pool);
                    return {
                        "(" + inner_c + ") + (" + leaf.code + ")",
                        IsBinary(TokenType::Plus, IsGrouping(inner_v), IsGrouping(leaf.verifier))
                    };
                }
            case 1:
                {
                    std::string p = "p_" + next_id();
                    return {"(" + inner_c + ")." + p, IsDot(IsGrouping(inner_v), p)};
                }
            case 2:
                {
                    return {"(" + inner_c + ")[0]", IsBracket(IsGrouping(inner_v), IsNumber("0"))};
                }
            case 3:
                {
                    std::string fn_name = "call_" + next_id();
                    return {fn_name + "(arg: " + inner_c + ")", IsCall(IsIdentifier(fn_name), {{"arg", inner_v}})};
                }
            case 4:
                {
                    return {
                        "if true then (" + inner_c + ") else (0)",
                        IsConditional(IsBoolean(true), IsGrouping(inner_v), IsGrouping(IsNumber("0")))
                    };
                }
            default:
                return {"(" + inner_c + ")", IsGrouping(inner_v)};
            }
        }

    private:
        std::pair<std::string, StmtVerifier> synth_statement()
        {
            int stmt_type = rand_range(0, 3);

            if (stmt_type == 0)
            {
                if (!ConstructRegistry::assignments().empty() && rand_range(0, 1) == 0)
                {
                    const auto& item = pick_random(ConstructRegistry::assignments());
                    return {item.code, item.verifier};
                }
                auto [e_c, e_v] = synth_expression();
                std::string var_name = next_id();
                auto [m_code, m_specs] = synth_modifiers(rand_range(0, 2));
                return {m_code + "let " + var_name + " = " + e_c, IsAssignment(m_specs, {{var_name}}, e_v)};
            }
            if (stmt_type == 1)
            {
                if (!ConstructRegistry::reassignments().empty() && rand_range(0, 1) == 0)
                {
                    const auto& item = pick_random(ConstructRegistry::reassignments());
                    return {item.code, item.verifier};
                }
                auto [e_c, e_v] = synth_expression();
                std::string var_name = "reassign_" + next_id();
                return {var_name + " = " + e_c, IsReassignment(IsIdentifier(var_name), e_v)};
            }
            if (stmt_type == 2)
            {
                if (!ConstructRegistry::expr_stmts().empty() && rand_range(0, 1) == 0)
                {
                    const auto& item = pick_random(ConstructRegistry::expr_stmts());
                    return {item.code, item.verifier};
                }
                auto [inner_c, inner_v] = synth_expression();
                std::string fn_name = "stmt_call_" + next_id();
                return {
                    fn_name + "(arg: " + inner_c + ")", IsExprStmt(IsCall(IsIdentifier(fn_name), {{"arg", inner_v}}))
                };
            }
            if (!ConstructRegistry::returns().empty() && rand_range(0, 1) == 0)
            {
                const auto& item = pick_random(ConstructRegistry::returns());
                return {item.code, item.verifier};
            }
            auto [e_c, e_v] = synth_expression();
            return {"return " + e_c, IsReturn({e_v})};
        }

        std::pair<std::string, ParamSpec> synth_parameter(bool allow_default)
        {
            auto [mods_c, mods_v] = synth_modifiers(rand_range(1, 2));
            auto [type_c, type_v] = synth_type();
            std::string name = "p_" + next_id();
            std::string code = mods_c + name + ": " + type_c;

            ExprVerifier def_v = nullptr;
            if (allow_default)
            {
                auto [e_c, e_v] = synth_expression();
                code += " = " + e_c;
                def_v = e_v;
            }
            return {code, {name, mods_v, type_v, def_v}};
        }

        std::pair<std::string, FieldSpec> synth_struct_field()
        {
            auto [mods_code, mods_specs] = synth_modifiers(rand_range(1, 2));
            auto [t_c, t_v] = synth_type();
            std::string f_name = "f_" + next_id();
            return {mods_code + f_name + ": " + t_c, {f_name, mods_specs, t_v}};
        }

        std::pair<std::string, EnumCaseSpec> synth_enum_case()
        {
            auto [mods_code, mods_specs] = synth_modifiers(rand_range(1, 2));
            std::string c_name = "C_" + next_id();

            std::string code = mods_code + c_name;
            ExprVerifier val_v = nullptr;

            if (rand_range(0, 1) == 0)
            {
                auto [e_c, e_v] = synth_expression();
                code += " = " + e_c;
                val_v = e_v;
            }
            return {code, {c_name, mods_specs, val_v}};
        }

        std::pair<std::string, SpecAdder> ctx_function()
        {
            auto ctxs = FunctionDefinitionContextsProvider::get_all();
            auto ctx = pick_random(ctxs);
            std::string name = "func_" + next_id();

            std::stringstream body_code;
            std::vector<StmtVerifier> body_specs;
            int stmt_count = rand_range(1, 4);
            for (int i = 0; i < stmt_count; ++i)
            {
                auto [s_code, s_spec] = synth_statement();
                body_code << "  " << s_code << "\n";
                body_specs.push_back(s_spec);
            }

            std::stringstream params_code;
            std::vector<ParamSpec> param_specs;
            int param_count = rand_range(0, 3);
            int default_start = rand_range(0, param_count);

            for (int i = 0; i < param_count; ++i)
            {
                auto [p_code, p_spec] = synth_parameter(i >= default_start);
                params_code << p_code;
                if (i < param_count - 1) params_code << ", ";
                param_specs.push_back(p_spec);
            }

            auto [mods_code, mods_specs] = synth_modifiers(rand_range(1, 3));
            std::string func_code = mods_code + "func " + name + "(" + params_code.str() + ") -> void {\n" + body_code.
                str() + "}\n";

            return {
                FunctionDefinitionContextsProvider::inject(ctx.source_template, func_code),
                [ctx, name, mods_specs, param_specs, body_specs](ProgramSpec& s)
                {
                    ctx.add_to_spec(s, IsFunctionDef(name, mods_specs, param_specs, {IsType("void")}, body_specs));
                }
            };
        }

        std::pair<std::string, SpecAdder> ctx_struct()
        {
            auto ctxs = StructDefinitionContextsProvider::get_all();
            auto ctx = pick_random(ctxs);
            std::string name = "Struct_" + next_id();

            std::stringstream body_code;
            std::vector<FieldSpec> field_specs;
            int field_count = rand_range(1, 3);
            for (int i = 0; i < field_count; ++i)
            {
                auto [f_code, f_spec] = synth_struct_field();
                body_code << "  " << f_code;
                body_code << (i < field_count - 1 ? ",\n" : "\n");
                field_specs.push_back(f_spec);
            }

            auto [mods_code, mods_specs] = synth_modifiers(rand_range(1, 3));
            std::string struct_code = mods_code + "struct " + name + " {\n" + body_code.str() + "}\n";

            return {
                StructDefinitionContextsProvider::inject(ctx.source_template, struct_code),
                [ctx, name, mods_specs, field_specs](ProgramSpec& s)
                {
                    ctx.add_to_spec(s, IsStructDef(name, mods_specs, field_specs));
                }
            };
        }

        std::pair<std::string, SpecAdder> ctx_enum()
        {
            auto ctxs = EnumDefinitionContextsProvider::get_all();
            auto ctx = pick_random(ctxs);
            std::string name = "Enum_" + next_id();

            auto [t_c, t_v] = synth_type();
            std::stringstream body_code;
            std::vector<EnumCaseSpec> case_specs;
            int case_count = rand_range(1, 3);
            for (int i = 0; i < case_count; ++i)
            {
                auto [c_code, c_spec] = synth_enum_case();
                body_code << "  " << c_code;
                body_code << (i < case_count - 1 ? ",\n" : "\n");
                case_specs.push_back(c_spec);
            }

            auto [mods_code, mods_specs] = synth_modifiers(rand_range(1, 3));
            std::string enum_code = mods_code + "enum " + name + ": " + t_c + " {\n" + body_code.str() + "}\n";

            return {
                EnumDefinitionContextsProvider::inject(ctx.source_template, enum_code),
                [ctx, name, mods_specs, t_v, case_specs](ProgramSpec& s)
                {
                    ctx.add_to_spec(s, IsEnumDef(name, mods_specs, t_v, case_specs));
                }
            };
        }

        template <typename T>
        void register_pool(const std::vector<RegistryEntry<T>>& pool,
                           const std::function<void(ProgramSpec&, const T&)>& adder)
        {
            if (pool.empty()) return;
            generators_.push_back([this, &pool, adder]() -> std::pair<std::string, SpecAdder>
            {
                const auto& item = pick_random(pool);
                return {item.code + "\n", [item, adder](ProgramSpec& s) { adder(s, item.verifier); }};
            });
        }

        void setup_generators()
        {
            generators_.emplace_back([this]() -> std::pair<std::string, SpecAdder>
            {
                auto ctx = pick_random(ExpressionContextsProvider::get_all());
                auto [e_c, e_v] = synth_expression();
                return {
                    ExpressionContextsProvider::inject(ctx.source_template, e_c),
                    [ctx, e_v](ProgramSpec& s) { ctx.add_to_spec(s, e_v); }
                };
            });

            generators_.emplace_back([this]() -> std::pair<std::string, SpecAdder>
            {
                auto ctx = pick_random(TypeAnnotationContextsProvider::get_all());
                auto [t_c, t_v] = synth_type();
                return {
                    TypeAnnotationContextsProvider::inject(ctx.source_template, t_c),
                    [ctx, t_v](ProgramSpec& s) { ctx.add_to_spec(s, t_v); }
                };
            });

            generators_.emplace_back([this]() -> std::pair<std::string, SpecAdder>
            {
                auto ctx = pick_random(AssignmentContextsProvider::get_all());
                std::string id = next_id();
                auto [e_c, e_v] = synth_expression();
                auto [m_c, m_v] = synth_modifiers(rand_range(1, 2));
                return {
                    AssignmentContextsProvider::inject(ctx.source_template, m_c + "let " + id + " = " + e_c),
                    [ctx, m_v, id, e_v](ProgramSpec& s) { ctx.add_to_spec(s, IsAssignment(m_v, {{id}}, e_v)); }
                };
            });

            generators_.emplace_back([this]() -> std::pair<std::string, SpecAdder>
            {
                auto ctx = pick_random(ReassignmentContextsProvider::get_all());
                std::string id = "reassign_" + next_id();
                auto [e_c, e_v] = synth_expression();
                return {
                    ReassignmentContextsProvider::inject(ctx.source_template, id + " = " + e_c),
                    [ctx, id, e_v](ProgramSpec& s) { ctx.add_to_spec(s, IsReassignment(IsIdentifier(id), e_v)); }
                };
            });

            generators_.emplace_back([this]() -> std::pair<std::string, SpecAdder>
            {
                auto ctx = pick_random(ExpressionStatementContextsProvider::get_all());
                std::string id = "stmt_call_" + next_id();
                auto [e_c, e_v] = synth_expression();
                ExprVerifier call_v = IsCall(IsIdentifier(id), {{"arg", e_v}});
                return {
                    ExpressionStatementContextsProvider::inject(ctx.source_template, id + "(arg: " + e_c + ")"),
                    [ctx, call_v](ProgramSpec& s) { ctx.add_to_spec(s, IsExprStmt(call_v)); }
                };
            });

            generators_.emplace_back([this]() -> std::pair<std::string, SpecAdder>
            {
                auto ctx = pick_random(ReturnStatementContextsProvider::get_all());
                auto [e_c, e_v] = synth_expression();
                return {
                    ReturnStatementContextsProvider::inject(ctx.source_template, "return " + e_c),
                    [ctx, e_v](ProgramSpec& s) { ctx.add_to_spec(s, IsReturn({e_v})); }
                };
            });

            generators_.emplace_back([this]() -> std::pair<std::string, SpecAdder>
            {
                auto ctx = pick_random(ImportContextsProvider::get_all());
                std::string path = "\"lib_" + next_id() + ".vs\"";
                return {
                    ImportContextsProvider::inject(ctx.source_template, "import " + path),
                    [ctx, path](ProgramSpec& s) { ctx.add_to_spec(s, IsImport(path)); }
                };
            });

            generators_.emplace_back([this]() -> std::pair<std::string, SpecAdder>
            {
                auto ctx = pick_random(DirectiveContextsProvider::get_all());
                std::string name = "dir_" + next_id();
                auto [e_c, e_v] = synth_expression();
                return {
                    DirectiveContextsProvider::inject(ctx.source_template, "#" + name + " = " + e_c),
                    [ctx, name, e_v](ProgramSpec& s) { ctx.add_to_spec(s, IsDirective(name, e_v)); }
                };
            });

            generators_.emplace_back([this]() -> std::pair<std::string, SpecAdder>
            {
                auto ctx = pick_random(TypeAliasContextsProvider::get_all());
                std::string name = "Alias_" + next_id();
                auto [t_c, t_v] = synth_type();
                auto [m_c, m_v] = synth_modifiers(rand_range(1, 3));
                return {
                    TypeAliasContextsProvider::inject(ctx.source_template, m_c + "typealias " + name + " = " + t_c),
                    [ctx, name, m_v, t_v](ProgramSpec& s) { ctx.add_to_spec(s, IsTypeAlias(name, m_v, t_v)); }
                };
            });

            generators_.emplace_back([this]() -> std::pair<std::string, SpecAdder>
            {
                auto ctx = pick_random(ModifierContextsProvider::get_all());
                auto [m_c, m_v] = synth_modifiers(rand_range(1, 3));
                return {
                    ModifierContextsProvider::inject(ctx.source_template, m_c),
                    [ctx, m_v](ProgramSpec& s) { ctx.add_to_spec(s, m_v); }
                };
            });

            generators_.emplace_back([this] { return ctx_function(); });
            generators_.emplace_back([this] { return ctx_struct(); });
            generators_.emplace_back([this] { return ctx_enum(); });

            register_pool<FuncVerifier>(ConstructRegistry::functions(), [](ProgramSpec& s, const FuncVerifier& v)
            {
                s.functions.emplace_back(v);
            });

            register_pool<StructVerifier>(ConstructRegistry::structs(), [](ProgramSpec& s, const StructVerifier& v)
            {
                s.structs.emplace_back(v);
            });

            register_pool<EnumVerifier>(ConstructRegistry::enums(), [](ProgramSpec& s, const EnumVerifier& v)
            {
                s.enums.emplace_back(v);
            });

            register_pool<AliasVerifier>(ConstructRegistry::aliases(), [](ProgramSpec& s, const AliasVerifier& v)
            {
                s.type_aliases.emplace_back(v);
            });

            register_pool<AssignmentVerifier>(ConstructRegistry::assignments(),
                                              [](ProgramSpec& s, const AssignmentVerifier& v)
                                              {
                                                  s.execution_steps.emplace_back(v);
                                              });

            register_pool<ReassignmentVerifier>(ConstructRegistry::reassignments(),
                                                [](ProgramSpec& s, const ReassignmentVerifier& v)
                                                {
                                                    s.execution_steps.emplace_back(v);
                                                });

            register_pool<DirectiveVerifier>(ConstructRegistry::directives(),
                                             [](ProgramSpec& s, const DirectiveVerifier& v)
                                             {
                                                 s.directives.emplace_back(v);
                                             });

            register_pool<ImportVerifier>(ConstructRegistry::imports(), [](ProgramSpec& s, const ImportVerifier& v)
            {
                s.imports.emplace_back(v);
            });

            register_pool<ExprStmtVerifier>(ConstructRegistry::expr_stmts(),
                                            [](ProgramSpec& s, const ExprStmtVerifier& v)
                                            {
                                                s.execution_steps.emplace_back(v);
                                            });
        }

    public:
        std::pair<std::string, ProgramSpec> generate_program(int piece_count)
        {
            std::vector<std::pair<std::string, SpecAdder>> pieces;
            pieces.reserve(piece_count);

            for (int i = 0; i < piece_count; ++i)
            {
                auto generator = pick_random(generators_);
                pieces.push_back(generator());
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
    };
}
