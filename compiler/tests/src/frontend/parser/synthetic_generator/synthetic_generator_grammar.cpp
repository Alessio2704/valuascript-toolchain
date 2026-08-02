#include "synthetic_generator.h"
#include "token/operator_lookup.h"

namespace valuascript::compiler::test
{
    static const std::vector<std::pair<TokenType, std::string>>& get_fuzzer_binary_ops()
    {
        static const auto ops = get_all_binary_operators();
        return ops;
    }

    static const std::vector<std::pair<TokenType, std::string>>& get_fuzzer_unary_ops()
    {
        static const auto ops = get_all_unary_operators();
        return ops;
    }

    void SyntheticGenerator::build_grammar()
    {
        auto dict_item_rule = [this](SyntheticGenerator& env, int depth) -> std::pair<std::string, DictItemSpec>
        {
            auto [m_c, m_v] = this->rule_modifiers(env, depth + 1);
            auto [val_c, val_v] = this->rule_expression(env, depth + 1);
            std::string key = "key_" + env.next_id();
            return {m_c + key + ": " + val_c, DictItemSpec{key, m_v, val_v}};
        };

        auto dict_elements_list = Repeat<DictItemSpec>(dict_item_rule,
                                                       [](const auto& c) { return c.sizes.dict_elements; }, ", ");

        auto switch_case_rule = [this](SyntheticGenerator& env, int depth) -> std::pair<std::string, SwitchCaseSpec>
        {
            auto [m_c, m_v] = this->rule_modifiers(env, depth + 1);
            auto [case_expr_c, case_expr_v] = this->rule_expression(env, depth + 1);

            int label_count = 1;
            if (env.roll_prob(env.get_config().features.switch_case_has_multiple_labels))
            {
                label_count = env.rand_range(env.get_config().sizes.switch_case_labels);
            }

            env.stats().expressions.total_switch_cases++;
            if (label_count > 1) env.stats().expressions.cases_with_multiple_labels++;

            std::string labels_code;
            std::vector<std::string> labels;
            for (int i = 0; i < label_count; ++i)
            {
                std::string label = "switch_case_" + env.next_id();
                labels.push_back(label);
                labels_code += label;
                if (i < label_count - 1) labels_code += ", ";
            }

            return {
                "  " + m_c + "case " + labels_code + " -> " + case_expr_c + "\n",
                SwitchCaseSpec(m_v, labels, case_expr_v)
            };
        };

        auto switch_cases_list = Repeat<SwitchCaseSpec>(switch_case_rule, [](const auto& c)
        {
            return c.sizes.switch_cases;
        }, "");

        auto expr_list_tuple = Repeat<ExprVerifier>(
            [this](SyntheticGenerator& env, int depth) { return this->rule_expression(env, depth + 1); },
            [](const auto& c) { return c.sizes.tuple_elements; },
            ", "
        );

        auto expr_list_tensor = Repeat<ExprVerifier>(
            [this](SyntheticGenerator& env, int depth) { return this->rule_expression(env, depth + 1); },
            [](const auto& c) { return c.sizes.tensor_elements; },
            ", "
        );

        rule_expression = WithRegistryFallback<ExprVerifier>(
            [this, dict_elements_list, switch_cases_list, expr_list_tuple, expr_list_tensor]
        (SyntheticGenerator& env, int depth) -> std::pair<std::string, ExprVerifier>
            {
                if (depth >= env.get_config().sizes.max_ast_depth)
                {
                    env.stats().expressions.leaf_fallback++;
                    return {"0", IsNumber("0")};
                }

                env.stats().max_ast_depth = std::max(env.stats().max_ast_depth, depth);

                auto type_idx = env.roll_weighted<ExpressionType>(
                    {
                        env.get_config().weights.expression_types.binary,
                        env.get_config().weights.expression_types.unary,
                        env.get_config().weights.expression_types.dot,
                        env.get_config().weights.expression_types.bracket,
                        env.get_config().weights.expression_types.call,
                        env.get_config().weights.expression_types.grouping,
                        env.get_config().weights.expression_types.switch_expr,
                        env.get_config().weights.expression_types.dict_expr,
                        env.get_config().weights.expression_types.tuple_expr,
                        env.get_config().weights.expression_types.tensor_expr,
                        env.get_config().weights.expression_types.conditional
                    }, ExpressionType::Grouping);

                auto [inner_c, inner_v] = this->rule_expression(env, depth + 1);

                switch (type_idx)
                {
                case ExpressionType::Binary:
                    {
                        env.stats().expressions.binary++;
                        auto [leaf_c, leaf_v] = this->rule_expression(env, depth + 1);
                        const auto& op = env.pick_random(get_fuzzer_binary_ops());
                        return {
                            "(" + inner_c + ") " + op.second + " (" + leaf_c + ")",
                            IsBinary(op.first, IsGrouping(inner_v), IsGrouping(leaf_v))
                        };
                    }
                case ExpressionType::Unary:
                    {
                        env.stats().expressions.unary++;
                        const auto& op = env.pick_random(get_fuzzer_unary_ops());
                        return {op.second + " (" + inner_c + ")", IsUnary(op.first, IsGrouping(inner_v))};
                    }
                case ExpressionType::Dot:
                    {
                        env.stats().expressions.dot++;
                        std::string prop = "prop_" + env.next_id();
                        return {"(" + inner_c + ")." + prop, IsDot(IsGrouping(inner_v), prop)};
                    }
                case ExpressionType::Bracket:
                    {
                        env.stats().expressions.bracket++;
                        auto [idx_c, idx_v] = this->rule_expression(env, depth + 1);
                        return {"(" + inner_c + ")[" + idx_c + "]", IsBracket(IsGrouping(inner_v), idx_v)};
                    }
                case ExpressionType::Call:
                    {
                        env.stats().expressions.call++;
                        std::string fn_name = "call_" + env.next_id();
                        return {fn_name + "(arg: " + inner_c + ")", IsCall(IsIdentifier(fn_name), {{"arg", inner_v}})};
                    }
                case ExpressionType::Switch:
                    {
                        env.stats().expressions.switch_expr++;
                        env.stats().expressions.switch_defaults++;
                        std::string code = "switch (" + inner_c + ") {\n";
                        auto [cases_c, cases_v] = switch_cases_list(env, depth);
                        code += cases_c;

                        auto [def_m_c, def_m_v] = this->rule_modifiers(env, depth + 1);
                        auto [def_expr_c, def_expr_v] = this->rule_expression(env, depth + 1);
                        code += "  " + def_m_c + "default -> " + def_expr_c + "\n}";
                        return {code, IsSwitch(inner_v, cases_v, def_m_v, def_expr_v)};
                    }
                case ExpressionType::Dict:
                    {
                        env.stats().expressions.dict_expr++;
                        auto [items_c, items_v] = dict_elements_list(env, depth);
                        std::string pad = items_v.empty() ? "" : " ";
                        return {"{" + pad + items_c + pad + "}", IsDict(items_v)};
                    }
                case ExpressionType::Tuple:
                    {
                        env.stats().expressions.tuple_expr++;
                        auto [elems_c, elems_v] = expr_list_tuple(env, depth);
                        return {"(" + elems_c + ")", IsTuple(elems_v)};
                    }
                case ExpressionType::Tensor:
                    {
                        env.stats().expressions.tensor_expr++;
                        auto [elems_c, elems_v] = expr_list_tensor(env, depth);
                        return {"[" + elems_c + "]", IsTensor(elems_v)};
                    }
                case ExpressionType::Conditional:
                    {
                        env.stats().expressions.conditional++;
                        auto [then_c, then_v] = this->rule_expression(env, depth + 1);
                        auto [else_c, else_v] = this->rule_expression(env, depth + 1);
                        return {
                            "if (" + inner_c + ") then (" + then_c + ") else (" + else_c + ")",
                            IsConditional(IsGrouping(inner_v), IsGrouping(then_v), IsGrouping(else_v))
                        };
                    }
                case ExpressionType::Grouping:
                default:
                    env.stats().expressions.grouping++;
                    return {"(" + inner_c + ")", IsGrouping(inner_v)};
                }
            },
            [](const auto& c) { return c.registry.expressions; },
            &ConstructRegistry::expressions()
        );

        auto mod_args_list = Repeat<ArgSpec>(
            [this](SyntheticGenerator& env, int depth) -> std::pair<std::string, ArgSpec>
            {
                std::string label = "arg_" + env.next_id();
                auto [e_c, e_v] = this->rule_expression(env, depth + 1);
                return {label + ": " + e_c, ArgSpec{label, e_v}};
            },
            [](const auto& c) { return c.sizes.modifier_arguments; },
            ", "
        );

        auto single_mod_rule = [mod_args_list](SyntheticGenerator& env,
                                               int depth) -> std::pair<std::string, ModifierSpec>
        {
            env.stats().modifiers.total_generated++;
            std::string name = "mod_" + env.next_id();
            auto [args_c, args_v] = mod_args_list(env, depth);

            if (!args_v.empty())
            {
                env.stats().modifiers.with_arguments++;
            }

            std::string code = "@" + name;
            if (!args_v.empty())
            {
                code += "(" + args_c + ")";
            }
            code += " ";
            return {code, ModifierSpec{name, args_v}};
        };

        auto raw_rule_modifiers = WithRegistryFallback<std::vector<ModifierSpec>>(
            Repeat<ModifierSpec>(single_mod_rule, [](const auto& c) { return c.sizes.modifiers_count; }, ""),
            [](const auto& c) { return c.registry.modifiers; },
            &ConstructRegistry::modifiers()
        );

        rule_modifiers = [raw_rule_modifiers](SyntheticGenerator& env, int depth)
        {
            auto res = raw_rule_modifiers(env, depth);
            if (!res.first.empty() && res.first.back() != ' ')
            {
                res.first += " ";
            }
            return res;
        };

        auto raw_rule_standalone_modifiers = WithRegistryFallback<std::vector<ModifierSpec>>(
            Repeat<ModifierSpec>(single_mod_rule, [](const auto& c) { return c.sizes.standalone_modifiers_count; }, ""),
            [](const auto& c) { return c.registry.modifiers; },
            &ConstructRegistry::modifiers()
        );

        rule_standalone_modifiers = [raw_rule_standalone_modifiers](SyntheticGenerator& env, int depth)
        {
            auto res = raw_rule_standalone_modifiers(env, depth);
            if (!res.first.empty() && res.first.back() != ' ')
            {
                res.first += " ";
            }
            return res;
        };

        rule_type = WithRegistryFallback<TypeVerifier>(
            [this](SyntheticGenerator& env, int depth) -> std::pair<std::string, TypeVerifier>
            {
                if (depth >= env.get_config().sizes.max_type_depth || env.roll_prob(
                    env.get_config().features.type_fallback_to_any))
                {
                    env.stats().types.fallback_to_any++;
                    return {"any", IsType("any")};
                }
                if (env.roll_prob(env.get_config().features.type_is_tuple_vs_generic))
                {
                    env.stats().types.tuple_types++;
                    auto [t1_c, t1_v] = this->rule_type(env, depth + 1);
                    auto [t2_c, t2_v] = this->rule_type(env, depth + 1);
                    return {"(" + t1_c + ", " + t2_c + ")", IsTupleType(t1_v, t2_v)};
                }
                env.stats().types.list_types++;
                auto [inner_c, inner_v] = this->rule_type(env, depth + 1);
                return {"List<" + inner_c + ">", IsType("List", inner_v)};
            },
            [](const auto& c) { return c.registry.types; },
            &ConstructRegistry::type_annotations()
        );

        auto opt_target_type = Opt<TypeVerifier>(
            [this](SyntheticGenerator& env, int depth) { return this->rule_type(env, depth); },
            [](const auto& c) { return c.features.assignment_has_explicit_type; }
        );

        auto rule_target = [this, opt_target_type](SyntheticGenerator& env,
                                                   int depth) -> std::pair<std::string, AssignmentTargetSpec>
        {
            std::string mods_c;
            std::vector<ModifierSpec> mods_v;

            if (env.roll_prob(env.get_config().features.assignment_target_has_modifiers))
            {
                auto res = this->rule_modifiers(env, depth);
                mods_c = res.first;
                mods_v = res.second;
            }

            std::string name = "multi_var_" + env.next_id();
            auto [t_c, t_v] = opt_target_type(env, depth);

            std::string code = mods_c + name;
            TypeVerifier tv = nullptr;
            if (t_v.has_value())
            {
                code += ": " + t_c;
                tv = t_v.value();
            }
            return {code, AssignmentTargetSpec(mods_v, name, tv)};
        };

        rule_assignment_targets = Repeat<AssignmentTargetSpec>(
            rule_target,
            [](const auto& c) { return c.sizes.multi_assign_targets; },
            ", "
        );

        GenRule<std::pair<ExprVerifier, ReassignTargetFlavor>> rule_reassign_target = [this](
            SyntheticGenerator& env, int depth)
            -> std::pair<std::string, std::pair<ExprVerifier, ReassignTargetFlavor>>
        {
            auto flavor = env.roll_weighted<ReassignTargetFlavor>(
                {
                    env.get_config().weights.reassign_target_flavors.id,
                    env.get_config().weights.reassign_target_flavors.dot,
                    env.get_config().weights.reassign_target_flavors.bracket,
                    env.get_config().weights.reassign_target_flavors.self_dot
                }, ReassignTargetFlavor::Id);

            if (flavor == ReassignTargetFlavor::Id)
            {
                env.stats().statements.reassignments_id++;
                std::string id = "reassign_id_" + env.next_id();
                return {id, {IsIdentifier(id), flavor}};
            }
            else if (flavor == ReassignTargetFlavor::Dot)
            {
                env.stats().statements.reassignments_dot++;
                std::string id = "reassign_obj_" + env.next_id();
                std::string prop = "reassign_prop_" + env.next_id();
                return {id + "." + prop, {IsDot(IsIdentifier(id), prop), flavor}};
            }
            else if (flavor == ReassignTargetFlavor::Bracket)
            {
                env.stats().statements.reassignments_bracket++;
                std::string id = "reassign_arr_" + env.next_id();
                auto [idx_c, idx_v] = this->rule_expression(env, depth + 1);
                return {id + "[" + idx_c + "]", {IsBracket(IsIdentifier(id), idx_v), flavor}};
            }
            else
            {
                env.stats().statements.reassignments_self++;
                std::string prop = "reassign_field_" + env.next_id();
                return {"self." + prop, {IsDot(IsSelf(), prop), flavor}};
            }
        };

        auto logic_stmt_rule = [this, opt_target_type, rule_reassign_target](
            SyntheticGenerator& env, int depth) -> std::pair<std::string, StmtVerifier>
        {
            auto stmt_type = env.roll_weighted<StatementType>({
                                                                  env.get_config().weights.statement_types.
                                                                      single_assign,
                                                                  env.get_config().weights.statement_types.multi_assign,
                                                                  env.get_config().weights.statement_types.reassign,
                                                                  env.get_config().weights.statement_types.expr_stmt
                                                              }, StatementType::SingleAssign);

            if (stmt_type == StatementType::SingleAssign)
            {
                env.stats().statements.single_assignments++;
                std::string stmt_m_code;
                std::vector<ModifierSpec> stmt_m_specs;
                if (env.roll_prob(env.get_config().features.assignment_has_let_modifiers))
                {
                    auto res = this->rule_modifiers(env, depth);
                    stmt_m_code = res.first;
                    stmt_m_specs = res.second;
                }

                std::string target_m_code;
                std::vector<ModifierSpec> target_m_specs;
                if (env.roll_prob(env.get_config().features.assignment_target_has_modifiers))
                {
                    auto res = this->rule_modifiers(env, depth);
                    target_m_code = res.first;
                    target_m_specs = res.second;
                }

                auto [e_c, e_v] = this->rule_expression(env, depth);
                std::string var_name = "single_var_" + env.next_id();

                auto [t_c, t_v] = opt_target_type(env, depth);

                bool has_outside = !stmt_m_specs.empty();
                bool has_inside = !target_m_specs.empty();

                if (has_outside && has_inside) env.stats().modifiers.attached_to_assignments_both++;
                else if (has_outside) env.stats().modifiers.attached_to_assignments_outside++;
                else if (has_inside) env.stats().modifiers.attached_to_assignments_inside++;

                if (t_v.has_value())
                {
                    env.stats().statements.explicit_type_annotations++;
                }

                std::vector<ModifierSpec> combined_mods = stmt_m_specs;
                combined_mods.insert(combined_mods.end(), target_m_specs.begin(), target_m_specs.end());

                std::string target_code = target_m_code + var_name;
                TypeVerifier tv = nullptr;
                if (t_v.has_value())
                {
                    target_code += ": " + t_c;
                    tv = t_v.value();
                }

                return {
                    stmt_m_code + "let " + target_code + " = " + e_c,
                    IsAssignment({{combined_mods, var_name, tv}}, e_v)
                };
            }
            if (stmt_type == StatementType::MultiAssign)
            {
                env.stats().statements.multi_assignments++;
                std::string stmt_m_code;
                std::vector<ModifierSpec> stmt_m_specs;
                if (env.roll_prob(env.get_config().features.assignment_has_let_modifiers))
                {
                    auto res = this->rule_modifiers(env, depth);
                    stmt_m_code = res.first;
                    stmt_m_specs = res.second;
                }

                auto [targets_c, targets_s] = this->rule_assignment_targets(env, depth);
                auto [e_c, e_v] = this->rule_expression(env, depth);

                bool has_outside = !stmt_m_specs.empty();
                bool has_inside = false;

                for (auto& target : targets_s)
                {
                    if (!target.modifiers.empty()) has_inside = true;
                    if (target.type_v != nullptr) env.stats().statements.explicit_type_annotations++;

                    std::vector<ModifierSpec> combined = stmt_m_specs;
                    combined.insert(combined.end(), target.modifiers.begin(), target.modifiers.end());
                    target.modifiers = combined;
                }

                if (has_outside && has_inside) env.stats().modifiers.attached_to_assignments_both++;
                else if (has_outside) env.stats().modifiers.attached_to_assignments_outside++;
                else if (has_inside) env.stats().modifiers.attached_to_assignments_inside++;

                return {stmt_m_code + "let " + targets_c + " = " + e_c, IsAssignment(targets_s, e_v)};
            }
            if (stmt_type == StatementType::Reassign)
            {
                auto [tgt_c, tgt_pair] = rule_reassign_target(env, depth);
                auto [val_c, val_v] = this->rule_expression(env, depth);
                return {tgt_c + " = " + val_c, IsReassignment(tgt_pair.first, val_v)};
            }

            env.stats().statements.expression_statements++;
            auto [e_c, e_v] = this->rule_expression(env, depth);
            std::string fn_name = "stmt_call_" + env.next_id();
            return {fn_name + "(arg: " + e_c + ")", IsExprStmt(IsCall(IsIdentifier(fn_name), {{"arg", e_v}}))};
        };

        rule_statement = [logic_stmt_rule](SyntheticGenerator& env,
                                           int depth) -> std::pair<std::string, StmtVerifier>
        {
            env.stats().total_nodes_generated++;
            if (env.roll_prob(env.get_config().registry.statements))
            {
                auto p = env.roll_weighted<HarvestStatementType>(
                    {
                        env.get_config().weights.harvest_statement_types.assignment,
                        env.get_config().weights.harvest_statement_types.reassignment,
                        env.get_config().weights.harvest_statement_types.expr_stmt
                    }, HarvestStatementType::Assignment);

                if (p == HarvestStatementType::Assignment && !ConstructRegistry::assignments().empty())
                {
                    env.stats().registry_fallbacks++;
                    auto& item = env.pick_random(ConstructRegistry::assignments());
                    return {item.code, StmtVerifier(item.verifier)};
                }
                else if (p == HarvestStatementType::Reassignment && !ConstructRegistry::reassignments().empty())
                {
                    env.stats().registry_fallbacks++;
                    auto& item = env.pick_random(ConstructRegistry::reassignments());
                    return {item.code, StmtVerifier(item.verifier)};
                }
                else if (p == HarvestStatementType::ExprStmt && !ConstructRegistry::expr_stmts().empty())
                {
                    env.stats().registry_fallbacks++;
                    auto& item = env.pick_random(ConstructRegistry::expr_stmts());
                    return {item.code, StmtVerifier(item.verifier)};
                }
            }
            env.stats().synthesized_from_scratch++;
            return logic_stmt_rule(env, depth);
        };

        rule_parameter = [this](SyntheticGenerator& env, int depth) -> std::pair<std::string, ParamSpec>
        {
            auto [mods_c, mods_v] = this->rule_modifiers(env, depth);
            auto [t_c, t_v] = this->rule_type(env, depth);
            std::string name = "param_" + env.next_id();
            std::string code = mods_c + name + ": " + t_c;

            if (!mods_v.empty())
            {
                env.stats().modifiers.attached_to_parameters++;
            }

            return {code, {name, mods_v, t_v, nullptr}};
        };

        auto logic_function_rule = [this](SyntheticGenerator& env, int depth) -> std::pair<std::string, FuncVerifier>
        {
            env.stats().functions.total++;
            std::string name = "func_" + env.next_id();

            std::string body_code;
            std::vector<StmtVerifier> body_specs;

            std::optional<std::string> doc_str = std::nullopt;
            if (env.roll_prob(env.get_config().features.func_has_docstring))
            {
                env.stats().functions.with_docstrings++;
                std::string content = "Synthetic docstring " + std::to_string(env.get_unique_id());
                doc_str = R"(""")" + content + R"(""")";
                body_code += "  " + *doc_str + "\n";
            }

            int stmt_count = env.rand_range(env.get_config().sizes.function_statements);
            env.stats().functions.total_body_statements += stmt_count;
            for (int i = 0; i < stmt_count; ++i)
            {
                auto [s_c, s_v] = this->rule_statement(env, depth);
                body_code += "  " + s_c + "\n";
                body_specs.push_back(s_v);
            }

            std::string params_code;
            std::vector<ParamSpec> param_specs;
            int param_count = env.rand_range(env.get_config().sizes.function_parameters);
            int default_start = env.rand_range(0, param_count);

            env.stats().functions.total_parameters += param_count;
            env.stats().functions.parameters_with_defaults += (param_count - default_start);

            for (int i = 0; i < param_count; ++i)
            {
                auto [p_c, p_v] = this->rule_parameter(env, depth);
                if (i >= default_start)
                {
                    auto [e_c, e_v] = this->rule_expression(env, depth);
                    p_c += " = " + e_c;
                    p_v.default_v = e_v;
                }
                params_code += p_c;
                if (i < param_count - 1) params_code += ", ";
                param_specs.push_back(p_v);
            }

            auto [mods_code, mods_specs] = this->rule_modifiers(env, depth);
            if (!mods_specs.empty())
            {
                env.stats().modifiers.attached_to_functions++;
            }

            std::string func_code = mods_code + "func " + name + "(" + params_code + ") -> void {\n" + body_code +
                "}\n";

            return {func_code, IsFunctionDef(name, mods_specs, param_specs, {IsType("void")}, body_specs, doc_str)};
        };

        rule_function = WithRegistryFallback<FuncVerifier>(
            logic_function_rule,
            [](const auto& c) { return c.registry.functions; },
            &ConstructRegistry::functions()
        );

        rule_struct_field = [this](SyntheticGenerator& env, int depth) -> std::pair<std::string, FieldSpec>
        {
            auto [mods_c, mods_v] = this->rule_modifiers(env, depth);
            std::string f_name = "struct_field_" + env.next_id();
            auto [t_c, t_v] = this->rule_type(env, depth);

            if (!mods_v.empty())
            {
                env.stats().modifiers.attached_to_struct_fields++;
            }

            return {
                mods_c + f_name + ": " + t_c,
                FieldSpec{f_name, mods_v, t_v}
            };
        };

        auto struct_fields_list = Repeat<FieldSpec>(
            rule_struct_field,
            [](const auto& c) { return c.sizes.struct_fields; },
            ",\n  "
        );

        auto logic_struct_rule = [this, struct_fields_list](SyntheticGenerator& env,
                                                            int depth) -> std::pair<std::string, StructVerifier>
        {
            env.stats().data_structures.total_structs++;
            auto [mods_c, mods_v] = this->rule_modifiers(env, depth);
            std::string name = "Struct_" + env.next_id();
            auto [fields_c, fields_v] = struct_fields_list(env, depth);

            env.stats().data_structures.total_fields += static_cast<int>(fields_v.size());
            if (!mods_v.empty())
            {
                env.stats().modifiers.attached_to_structs++;
            }

            std::string code = mods_c + "struct " + name + " {\n";
            if (!fields_v.empty()) code += "  " + fields_c + "\n";
            code += "}\n";

            return {code, IsStructDef(name, mods_v, fields_v)};
        };

        rule_struct = WithRegistryFallback<StructVerifier>(
            logic_struct_rule,
            [](const auto& c) { return c.registry.structs; },
            &ConstructRegistry::structs()
        );

        auto logic_extension_rule = [this](SyntheticGenerator& env, int depth) -> std::pair<std::string, ExtVerifier>
        {
            auto [mods_c, mods_v] = this->rule_modifiers(env, depth);
            auto [t_c, t_v] = this->rule_type(env, depth);

            std::string code = mods_c + "extension " + t_c + " {}\n";
            return {code, IsExtensionDef(mods_v, t_v, {})};
        };

        rule_extension = WithRegistryFallback<ExtVerifier>(
            logic_extension_rule,
            [](const auto& c) { return c.registry.extensions; },
            &ConstructRegistry::extensions()
        );

        auto opt_enum_value = Opt<ExprVerifier>(
            [this](SyntheticGenerator& env, int depth) { return this->rule_expression(env, depth); },
            [](const auto& c) { return c.features.enum_case_has_value; }
        );

        rule_enum_case = [this, opt_enum_value](SyntheticGenerator& env,
                                                int depth) -> std::pair<std::string, EnumCaseSpec>
        {
            auto [mods_c, mods_v] = this->rule_modifiers(env, depth);
            std::string name = "enum_case_" + env.next_id();
            auto [val_c, val_v] = opt_enum_value(env, depth);

            if (!mods_v.empty())
            {
                env.stats().modifiers.attached_to_enum_cases++;
            }

            std::string code = mods_c + name;
            ExprVerifier v = nullptr;
            if (val_v.has_value())
            {
                env.stats().data_structures.cases_with_values++;
                code += " = " + val_c;
                v = val_v.value();
            }
            return {code, EnumCaseSpec{name, mods_v, v}};
        };

        auto enum_cases_list = Repeat<EnumCaseSpec>(
            rule_enum_case,
            [](const auto& c) { return c.sizes.enum_cases; },
            ",\n  "
        );

        auto logic_enum_rule = [this, enum_cases_list](SyntheticGenerator& env,
                                                       int depth) -> std::pair<std::string, EnumVerifier>
        {
            env.stats().data_structures.total_enums++;
            auto [mods_c, mods_v] = this->rule_modifiers(env, depth);
            std::string name = "Enum_" + env.next_id();
            auto [t_c, t_v] = this->rule_type(env, depth);
            auto [cases_c, cases_v] = enum_cases_list(env, depth);

            env.stats().data_structures.total_cases += static_cast<int>(cases_v.size());
            if (!mods_v.empty())
            {
                env.stats().modifiers.attached_to_enums++;
            }

            std::string code = mods_c + "enum " + name + ": " + t_c + " {\n";
            if (!cases_v.empty()) code += "  " + cases_c + "\n";
            code += "}\n";

            return {code, IsEnumDef(name, mods_v, t_v, cases_v)};
        };

        rule_enum = WithRegistryFallback<EnumVerifier>(
            logic_enum_rule,
            [](const auto& c) { return c.registry.enums; },
            &ConstructRegistry::enums()
        );

        auto logic_alias_rule = [this](SyntheticGenerator& env, int depth) -> std::pair<std::string, AliasVerifier>
        {
            auto [mods_c, mods_v] = this->rule_modifiers(env, depth);
            std::string name = "Alias_" + env.next_id();
            auto [t_c, t_v] = this->rule_type(env, depth);
            return {
                mods_c + "typealias " + name + " = " + t_c,
                IsTypeAlias(name, mods_v, t_v)
            };
        };

        rule_type_alias = WithRegistryFallback<AliasVerifier>(
            logic_alias_rule,
            [](const auto& c) { return c.registry.type_aliases; },
            &ConstructRegistry::aliases()
        );

        auto logic_import_rule = [this](SyntheticGenerator& env, int depth) -> std::pair<std::string, ImportVerifier>
        {
            auto [mods_c, mods_v] = this->rule_modifiers(env, depth);
            std::string path = R"("lib_)" + env.next_id() + R"(.vs")";
            return {
                mods_c + "import " + path,
                IsImport(path, mods_v)
            };
        };

        rule_import = WithRegistryFallback<ImportVerifier>(
            logic_import_rule,
            [](const auto& c) { return c.registry.imports; },
            &ConstructRegistry::imports()
        );

        auto opt_dir_value = Opt<ExprVerifier>(
            [this](SyntheticGenerator& env, int depth) { return this->rule_expression(env, depth); },
            [](const auto& c) { return c.features.directive_has_value; }
        );

        auto logic_directive_rule = [opt_dir_value](SyntheticGenerator& env,
                                                    int depth) -> std::pair<std::string, DirectiveVerifier>
        {
            std::string name = "dir_" + env.next_id();
            auto [val_c, val_v] = opt_dir_value(env, depth);

            if (val_v.has_value())
            {
                return {"#" + name + " = " + val_c, IsDirective(name, val_v.value())};
            }
            return {"#" + name, IsDirective(name, IsNull())};
        };

        rule_directive = WithRegistryFallback<DirectiveVerifier>(
            logic_directive_rule,
            [](const auto& c) { return c.registry.directives; },
            &ConstructRegistry::directives()
        );

        auto logic_return_rule = [this](SyntheticGenerator& env, int depth) -> std::pair<std::string, ReturnVerifier>
        {
            auto [m_code, m_specs] = this->rule_standalone_modifiers(env, depth);

            int count = 1;
            if (env.roll_prob(env.get_config().features.return_has_multiple_values))
            {
                count = env.rand_range(env.get_config().sizes.return_values);
            }

            env.stats().statements.total_returns++;
            env.stats().statements.total_return_values += count;
            if (count > 1) env.stats().statements.multi_returns++;

            std::string expr_code;
            std::vector<ExprVerifier> expr_verifiers;
            for (int i = 0; i < count; ++i)
            {
                auto [e_code, e_verifier] = this->rule_expression(env, depth);
                expr_code += e_code;
                expr_verifiers.push_back(e_verifier);
                if (i < count - 1) expr_code += ", ";
            }

            return {
                m_code + "return " + expr_code + "\n",
                ReturnVerifier(IsReturn(m_specs, expr_verifiers))
            };
        };

        rule_return = WithRegistryFallback<ReturnVerifier>(
            logic_return_rule,
            [](const auto& c) { return c.registry.returns; },
            &ConstructRegistry::returns()
        );
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

        register_pyramid_member<ExprVerifier>(
            TopLevelConstruct::Expression, InjectableType::Expression,
            [this] { return this->rule_expression(*this, 0); });

        register_pyramid_member<TypeVerifier>(
            TopLevelConstruct::TypeAnnotation, InjectableType::TypeAnnotation,
            [this] { return this->rule_type(*this, 0); });

        register_pyramid_member<StmtVerifier>(
            TopLevelConstruct::Statement, InjectableType::StrongStatement,
            [this] { return this->rule_statement(*this, 0); });

        register_pyramid_member<ModifierVerifier>(
            TopLevelConstruct::Modifier, InjectableType::Modifier,
            [this] { return this->rule_standalone_modifiers(*this, 0); });

        register_pyramid_member<ReturnVerifier>(
            TopLevelConstruct::ReturnStmt, InjectableType::WeakStatement,
            [this] { return this->rule_return(*this, 0); });

        register_top_level<FuncVerifier>(
            TopLevelConstruct::FunctionDef,
            [this] { return this->rule_function(*this, 0); });

        register_top_level<ExtVerifier>(
            TopLevelConstruct::ExtensionDef,
            [this] { return this->rule_extension(*this, 0); });

        register_top_level<StructVerifier>(
            TopLevelConstruct::StructDef,
            [this] { return this->rule_struct(*this, 0); });

        register_top_level<EnumVerifier>(
            TopLevelConstruct::EnumDef,
            [this] { return this->rule_enum(*this, 0); });

        register_top_level<AliasVerifier>(
            TopLevelConstruct::TypeAlias,
            [this] { return this->rule_type_alias(*this, 0); });

        register_top_level<ImportVerifier>(
            TopLevelConstruct::ImportStmt,
            [this] { return this->rule_import(*this, 0); });

        register_top_level<DirectiveVerifier>(
            TopLevelConstruct::Directive,
            [this] { return this->rule_directive(*this, 0); });
    }
}
