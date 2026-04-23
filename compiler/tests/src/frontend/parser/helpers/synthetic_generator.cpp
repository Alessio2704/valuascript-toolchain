#include "synthetic_generator.h"
#include <sstream>
#include <algorithm>

namespace valuascript::compiler::test
{
    SyntheticGenerator::SyntheticGenerator(size_t seed, SyntheticGeneratorConfig config)
        : rng_(seed), config_(std::move(config))
    {
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
        int result = roll_weighted<int>({
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

    std::pair<std::string, ParamSpec> SyntheticGenerator::synth_parameter(bool allow_default)
    {
        auto [mods_c, mods_v] = synth_modifiers(rand_range(config_.sizes.modifiers_count));
        auto [type_c, type_v] = synth_type();
        std::string name = "param_" + next_id();
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

    std::pair<std::string, FieldSpec> SyntheticGenerator::synth_struct_field()
    {
        auto [mods_code, mods_specs] = synth_modifiers(rand_range(config_.sizes.modifiers_count));
        auto [t_c, t_v] = synth_type();
        std::string f_name = "struct_field_" + next_id();
        return {mods_code + f_name + ": " + t_c, {f_name, mods_specs, t_v}};
    }

    std::pair<std::string, EnumCaseSpec> SyntheticGenerator::synth_enum_case()
    {
        auto [mods_code, mods_specs] = synth_modifiers(rand_range(config_.sizes.modifiers_count));
        std::string c_name = "enum_case_" + next_id();
        std::string code = mods_code + c_name;
        ExprVerifier val_v = nullptr;

        if (roll_prob(config_.features.enum_case_has_value))
        {
            auto [e_c, e_v] = synth_expression();
            code += " = " + e_c;
            val_v = e_v;
        }
        return {code, {c_name, mods_specs, val_v}};
    }

    std::pair<std::string, std::vector<AssignmentTargetSpec>> SyntheticGenerator::synth_assignment_targets(int count)
    {
        std::string code;
        std::vector<AssignmentTargetSpec> specs;
        for (int i = 0; i < count; ++i)
        {
            std::string target_name = "multi_var_" + next_id();
            std::string target_code = target_name;
            TypeVerifier t_v = nullptr;
            if (roll_prob(config_.features.assignment_has_explicit_type))
            {
                auto [t_c, t_verifier] = synth_type();
                target_code += ": " + t_c;
                t_v = t_verifier;
            }
            code += target_code;
            specs.push_back({target_name, t_v});
            if (i < count - 1) code += ", ";
        }
        return {code, specs};
    }

    std::pair<std::string, StmtVerifier> SyntheticGenerator::logic_synth_statement()
    {
        auto stmt_type = roll_weighted<StatementType>({
                                                          config_.weights.statement_types.single_assign,
                                                          config_.weights.statement_types.multi_assign,
                                                          config_.weights.statement_types.reassign,
                                                          config_.weights.statement_types.expr_stmt
                                                      }, StatementType::SingleAssign);

        auto [m_code, m_specs] = synth_modifiers(rand_range(config_.sizes.modifiers_count));

        if (stmt_type == StatementType::SingleAssign)
        {
            auto [e_c, e_v] = synth_expression();
            std::string var_name = "single_var_" + next_id();
            return {m_code + "let " + var_name + " = " + e_c, IsAssignment(m_specs, {{var_name}}, e_v)};
        }
        if (stmt_type == StatementType::MultiAssign)
        {
            int count = rand_range(config_.sizes.multi_assign_targets);
            auto [targets_c, targets_s] = synth_assignment_targets(count);
            auto [e_c, e_v] = synth_expression();
            return {m_code + "let " + targets_c + " = " + e_c, IsAssignment(m_specs, targets_s, e_v)};
        }
        if (stmt_type == StatementType::Reassign)
        {
            auto [val_c, val_v] = synth_expression();
            auto target_flavor = roll_weighted<ReassignTargetFlavor>({
                                                                         config_.weights.reassign_target_flavors.id,
                                                                         config_.weights.reassign_target_flavors.dot,
                                                                         config_.weights.reassign_target_flavors.
                                                                         bracket,
                                                                         config_.weights.reassign_target_flavors.
                                                                         self_dot
                                                                     }, ReassignTargetFlavor::Id);

            if (target_flavor == ReassignTargetFlavor::Id)
            {
                std::string id = "reassign_id_" + next_id();
                return {id + " = " + val_c, IsReassignment(IsIdentifier(id), val_v)};
            }
            else if (target_flavor == ReassignTargetFlavor::Dot)
            {
                std::string id = "reassign_obj_" + next_id();
                std::string prop = "reassign_prop_" + next_id();
                return {id + "." + prop + " = " + val_c, IsReassignment(IsDot(IsIdentifier(id), prop), val_v)};
            }
            else if (target_flavor == ReassignTargetFlavor::Bracket)
            {
                std::string id = "reassign_arr_" + next_id();
                return {id + "[0] = " + val_c, IsReassignment(IsBracket(IsIdentifier(id), IsNumber("0")), val_v)};
            }
            else
            {
                std::string prop = "reassign_field_" + next_id();
                return {"self." + prop + " = " + val_c, IsReassignment(IsDot(IsSelf(), prop), val_v)};
            }
        }

        auto [e_c, e_v] = synth_expression();
        std::string fn_name = "stmt_call_" + next_id();
        return {fn_name + "(arg: " + e_c + ")", IsExprStmt(IsCall(IsIdentifier(fn_name), {{"arg", e_v}}))};
    }

    std::pair<std::string, FuncVerifier> SyntheticGenerator::logic_synth_function()
    {
        std::string name = "func_" + next_id();
        std::stringstream body_code;
        std::vector<StmtVerifier> body_specs;

        std::optional<std::string> doc_str = std::nullopt;
        if (roll_prob(config_.features.func_has_docstring))
        {
            std::string content = "Synthetic docstring " + std::to_string(unique_id_);
            doc_str = R"(""")" + content + R"(""")";
            body_code << "  " << *doc_str << "\n";
        }

        int stmt_count = rand_range(config_.sizes.function_statements);
        for (int i = 0; i < stmt_count; ++i)
        {
            auto [s_code, s_spec] = synth_statement();
            body_code << "  " << s_code << "\n";
            body_specs.push_back(s_spec);
        }

        std::stringstream params_code;
        std::vector<ParamSpec> param_specs;
        int param_count = rand_range(config_.sizes.function_parameters);
        int default_start = rand_range(0, param_count);

        for (int i = 0; i < param_count; ++i)
        {
            auto [p_code, p_spec] = synth_parameter(i >= default_start);
            params_code << p_code;
            if (i < param_count - 1) params_code << ", ";
            param_specs.push_back(p_spec);
        }

        auto [mods_code, mods_specs] = synth_modifiers(rand_range(config_.sizes.modifiers_count));
        std::string func_code = mods_code + "func " + name + "(" + params_code.str() + ") -> void {\n" + body_code.str()
            + "}\n";
        return {func_code, IsFunctionDef(name, mods_specs, param_specs, {IsType("void")}, body_specs, doc_str)};
    }

    std::pair<std::string, StructVerifier> SyntheticGenerator::logic_synth_struct()
    {
        std::string name = "Struct_" + next_id();
        std::stringstream body_code;
        std::vector<FieldSpec> field_specs;
        int field_count = rand_range(config_.sizes.struct_fields);
        for (int i = 0; i < field_count; ++i)
        {
            auto [f_code, f_spec] = synth_struct_field();
            body_code << "  " << f_code << (i < field_count - 1 ? ",\n" : "\n");
            field_specs.push_back(f_spec);
        }
        auto [mods_code, mods_specs] = synth_modifiers(rand_range(config_.sizes.modifiers_count));
        return {
            mods_code + "struct " + name + " {\n" + body_code.str() + "}\n",
            IsStructDef(name, mods_specs, field_specs)
        };
    }

    std::pair<std::string, EnumVerifier> SyntheticGenerator::logic_synth_enum()
    {
        std::string name = "Enum_" + next_id();
        auto [t_c, t_v] = synth_type();
        std::stringstream body_code;
        std::vector<EnumCaseSpec> case_specs;
        int case_count = rand_range(config_.sizes.enum_cases);
        for (int i = 0; i < case_count; ++i)
        {
            auto [c_code, c_spec] = synth_enum_case();
            body_code << "  " << c_code << (i < case_count - 1 ? ",\n" : "\n");
            case_specs.push_back(c_spec);
        }
        auto [mods_code, mods_specs] = synth_modifiers(rand_range(config_.sizes.modifiers_count));
        return {
            mods_code + "enum " + name + ": " + t_c + " {\n" + body_code.str() + "}\n",
            IsEnumDef(name, mods_specs, t_v, case_specs)
        };
    }

    std::pair<std::string, AliasVerifier> SyntheticGenerator::logic_synth_type_alias()
    {
        std::string name = "Alias_" + next_id();
        auto [t_c, t_v] = synth_type();
        auto [mods_c, mods_v] = synth_modifiers(rand_range(config_.sizes.modifiers_count));
        return {
            mods_c + "typealias " + name + " = " + t_c,
            IsTypeAlias(name, mods_v, t_v)
        };
    }

    std::pair<std::string, ImportVerifier> SyntheticGenerator::logic_synth_import()
    {
        std::string path = R"("lib_)" + next_id() + R"(.vs")";
        return {"import " + path, IsImport(path)};
    }

    std::pair<std::string, DirectiveVerifier> SyntheticGenerator::logic_synth_directive()
    {
        std::string name = "dir_" + next_id();
        if (roll_prob(config_.features.directive_has_value))
        {
            auto [e_c, e_v] = synth_expression();
            return {"#" + name + " = " + e_c, IsDirective(name, e_v)};
        }
        return {"#" + name, IsDirective(name, IsNull())};
    }

    std::pair<std::string, StmtVerifier> SyntheticGenerator::harvest_statement()
    {
        auto p = roll_weighted<HarvestStatementType>({
                                                         config_.weights.harvest_statement_types.assignment,
                                                         config_.weights.harvest_statement_types.reassignment,
                                                         config_.weights.harvest_statement_types.expr_stmt
                                                     }, HarvestStatementType::Assignment);

        if (p == HarvestStatementType::Assignment && !ConstructRegistry::assignments().empty())
        {
            auto& item = pick_random(ConstructRegistry::assignments());
            return {item.code, StmtVerifier(item.verifier)};
        }
        else if (p == HarvestStatementType::Reassignment && !ConstructRegistry::reassignments().empty())
        {
            auto& item = pick_random(ConstructRegistry::reassignments());
            return {item.code, StmtVerifier(item.verifier)};
        }
        else if (p == HarvestStatementType::ExprStmt && !ConstructRegistry::expr_stmts().empty())
        {
            auto& item = pick_random(ConstructRegistry::expr_stmts());
            return {item.code, StmtVerifier(item.verifier)};
        }
        return logic_synth_statement();
    }

    std::pair<std::string, std::vector<ModifierSpec>> SyntheticGenerator::synth_modifiers(int count)
    {
        const auto& pool = ConstructRegistry::modifiers();
        if (!pool.empty() && roll_prob(config_.registry.modifiers))
        {
            const auto& item = pick_random(pool);
            return {item.code + " ", item.verifier};
        }

        std::string code;
        std::vector<ModifierSpec> specs;
        if (count <= 0) return {"", {}};

        for (int i = 0; i < count; ++i)
        {
            std::string mod_name = "mod_" + next_id();
            std::vector<ArgSpec> args;
            std::string arg_code;
            int arg_count = rand_range(config_.sizes.modifier_arguments);
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
        return {code, specs};
    }

    std::pair<std::string, TypeVerifier> SyntheticGenerator::synth_type(int depth)
    {
        const auto& pool = ConstructRegistry::type_annotations();
        if (!pool.empty() && roll_prob(config_.registry.types))
        {
            const auto& itm = pick_random(pool);
            return {itm.code, itm.verifier};
        }
        if (depth >= 2 || roll_prob(config_.features.type_fallback_to_any)) return {"any", IsType("any")};
        if (roll_prob(config_.features.type_is_tuple_vs_generic))
        {
            auto [t1_c, t1_v] = synth_type(depth + 1);
            auto [t2_c, t2_v] = synth_type(depth + 1);
            return {"(" + t1_c + ", " + t2_c + ")", IsTupleType({t1_v, t2_v})};
        }
        auto [inner_c, inner_v] = synth_type(depth + 1);
        return {"List<" + inner_c + ">", IsType("List", {inner_v})};
    }

    std::pair<std::string, ExprVerifier> SyntheticGenerator::synth_expression(int depth, int max_depth)
    {
        const auto& pool = ConstructRegistry::expressions();
        if (!pool.empty() && roll_prob(config_.registry.expressions))
        {
            const auto& itm = pick_random(pool);
            return {itm.code, itm.verifier};
        }
        if (depth >= max_depth) return {"0", IsNumber("0")};
        auto [inner_c, inner_v] = synth_expression(depth + 1, max_depth);

        auto type_idx = roll_weighted<ExpressionType>({
                                                          config_.weights.expression_types.binary,
                                                          config_.weights.expression_types.dot,
                                                          config_.weights.expression_types.bracket,
                                                          config_.weights.expression_types.call,
                                                          config_.weights.expression_types.grouping
                                                      }, ExpressionType::Grouping);

        switch (type_idx)
        {
        case ExpressionType::Binary:
            {
                auto [leaf_c, leaf_v] = synth_expression(depth + 1, max_depth);
                return {
                    "(" + inner_c + ") + (" + leaf_c + ")",
                    IsBinary(TokenType::Plus, IsGrouping(inner_v), IsGrouping(leaf_v))
                };
            }
        case ExpressionType::Dot:
            {
                std::string prop_name = "prop_" + next_id();
                return {"(" + inner_c + ")." + prop_name, IsDot(IsGrouping(inner_v), prop_name)};
            }
        case ExpressionType::Bracket:
            {
                return {"(" + inner_c + ")[0]", IsBracket(IsGrouping(inner_v), IsNumber("0"))};
            }
        case ExpressionType::Call:
            {
                std::string fn_name = "call_" + next_id();
                return {fn_name + "(arg: " + inner_c + ")", IsCall(IsIdentifier(fn_name), {{"arg", inner_v}})};
            }
        case ExpressionType::Grouping:
        default:
            return {"(" + inner_c + ")", IsGrouping(inner_v)};
        }
    }

    std::pair<std::string, StmtVerifier> SyntheticGenerator::synth_statement()
    {
        if (roll_prob(config_.registry.statements)) return harvest_statement();
        return logic_synth_statement();
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
        UniversalVerifier final_verifier = wrapper_ctx.transform_verifier(transformed_verifier);

        return {
            wrapped_code, [final_verifier](ProgramSpec& s)
            {
                std::visit([&](auto&& v) { SpecAdder::add(s, v); }, final_verifier);
            }
        };
    }

    void SyntheticGenerator::setup_generators()
    {
        generators_[static_cast<int>(TopLevelConstruct::None)] = []() -> std::pair<std::string, SpecAdderFn>
        {
            return {
                "", [](ProgramSpec&)
                {
                }
            };
        };

        register_pyramid_member<ExprVerifier, ExprVerifier>(
            TopLevelConstruct::Expression, InjectableType::Expression,
            config_.registry.expressions, ConstructRegistry::expressions(),
            [this] { return synth_expression(); });

        register_pyramid_member<TypeVerifier, TypeVerifier>(
            TopLevelConstruct::TypeAnnotation, InjectableType::TypeAnnotation,
            config_.registry.types, ConstructRegistry::type_annotations(),
            [this] { return synth_type(); });

        register_pyramid_member<StmtVerifier, AssignmentVerifier>(
            TopLevelConstruct::Statement, InjectableType::StrongStatement,
            config_.registry.statements, ConstructRegistry::assignments(),
            [this] { return synth_statement(); });

        register_pyramid_member<ModifierVerifier, ModifierVerifier>(
            TopLevelConstruct::Modifier, InjectableType::Modifier,
            config_.registry.modifiers, ConstructRegistry::modifiers(),
            [this] { return synth_modifiers(rand_range(config_.sizes.standalone_modifiers_count)); });

        register_pyramid_member<ReturnVerifier, ReturnVerifier>(
            TopLevelConstruct::ReturnStmt,
            InjectableType::WeakStatement,
            config_.registry.returns,
            ConstructRegistry::returns(), [this]
            {
                auto [e_code, e_verifier] = synth_expression();
                return std::make_pair(
                    "return " + e_code + "\n",
                    ReturnVerifier(IsReturn({e_verifier}))
                );
            }
        );

        register_hybrid_generator<FuncVerifier, FuncVerifier>(
            TopLevelConstruct::FunctionDef, config_.registry.functions,
            ConstructRegistry::functions(),
            [this] { return logic_synth_function(); });

        register_hybrid_generator<StructVerifier, StructVerifier>(
            TopLevelConstruct::StructDef, config_.registry.structs,
            ConstructRegistry::structs(), [this] { return logic_synth_struct(); });

        register_hybrid_generator<EnumVerifier, EnumVerifier>(
            TopLevelConstruct::EnumDef, config_.registry.enums, ConstructRegistry::enums(),
            [this] { return logic_synth_enum(); });

        register_hybrid_generator<AliasVerifier, AliasVerifier>(
            TopLevelConstruct::TypeAlias, config_.registry.type_aliases,
            ConstructRegistry::aliases(),
            [this] { return logic_synth_type_alias(); });

        register_hybrid_generator<ImportVerifier, ImportVerifier>(
            TopLevelConstruct::ImportStmt, config_.registry.imports,
            ConstructRegistry::imports(),
            [this] { return logic_synth_import(); });

        register_hybrid_generator<DirectiveVerifier, DirectiveVerifier>(
            TopLevelConstruct::Directive, config_.registry.directives,
            ConstructRegistry::directives(), [this] { return logic_synth_directive(); });
    }

    std::pair<std::string, ProgramSpec> SyntheticGenerator::generate_program(int piece_count)
    {
        std::vector<std::pair<std::string, SpecAdderFn>> pieces;
        pieces.reserve(piece_count);

        for (int i = 0; i < piece_count; ++i)
        {
            TopLevelConstruct choice = roll_top_level_construct();
            if (choice == TopLevelConstruct::None) continue;
            pieces.push_back(generators_[static_cast<int>(choice)]());
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
