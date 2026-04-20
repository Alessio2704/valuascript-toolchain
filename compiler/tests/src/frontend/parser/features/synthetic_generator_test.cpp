#include <gtest/gtest.h>
#include <unordered_set>
#include <string>

#include "frontend/parser/helpers/synthetic_generator.h"
#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class SyntheticGeneratorKnobTest : public ParserTestBase
    {
    protected:
        static size_t get_iterations()
        {
            if (const char* env_p = std::getenv("FUZZ_ITERATIONS"))
            {
                return std::stoul(env_p);
            }
            return 10000;
        }

        static constexpr double MARGIN = 0.02;

        static std::shared_ptr<Program> parse(const std::string& code)
        {
            CompilerContext context;
            context.settings.fail_fast = true;
            try
            {
                return run_parser(code, context);
            }
            catch (...)
            {
                return nullptr;
            }
        }

        static void zero_all_weights(SyntheticGeneratorConfig& config)
        {
            config.weights.top_level_constructs.expression = 0.0;
            config.weights.top_level_constructs.type_annotation = 0.0;
            config.weights.top_level_constructs.statement = 0.0;
            config.weights.top_level_constructs.return_stmt = 0.0;
            config.weights.top_level_constructs.modifier = 0.0;
            config.weights.top_level_constructs.function_def = 0.0;
            config.weights.top_level_constructs.struct_def = 0.0;
            config.weights.top_level_constructs.enum_def = 0.0;
            config.weights.top_level_constructs.type_alias = 0.0;
            config.weights.top_level_constructs.import_stmt = 0.0;
            config.weights.top_level_constructs.directive = 0.0;

            config.weights.statement_types.single_assign = 0.0;
            config.weights.statement_types.multi_assign = 0.0;
            config.weights.statement_types.reassign = 0.0;
            config.weights.statement_types.expr_stmt = 0.0;
        }
    };

    TEST_F(SyntheticGeneratorKnobTest, ExclusiveSynthesisAndStrictModifiers)
    {
        struct TestGoal
        {
            TopLevelConstruct top_type;
            std::optional<StatementType> stmt_type = std::nullopt;
            std::string label;
        };

        std::vector<TestGoal> goals = {
            {TopLevelConstruct::FunctionDef, std::nullopt, "Function"},
            {TopLevelConstruct::StructDef, std::nullopt, "Struct"},
            {TopLevelConstruct::EnumDef, std::nullopt, "Enum"},
            {TopLevelConstruct::TypeAlias, std::nullopt, "Alias"},
            {TopLevelConstruct::ImportStmt, std::nullopt, "Import"},
            {TopLevelConstruct::Directive, std::nullopt, "Directive"},
            {TopLevelConstruct::ReturnStmt, std::nullopt, "Return"},
            {TopLevelConstruct::Statement, StatementType::SingleAssign, "SingleLet"},
            {TopLevelConstruct::Statement, StatementType::MultiAssign, "MultiLet"},
            {TopLevelConstruct::Statement, StatementType::Reassign, "Reassign"},
            {TopLevelConstruct::Statement, StatementType::ExprStmt, "ExprStmt"}
        };

        for (const auto& goal : goals)
        {
            for (size_t mod_count = 0; mod_count <= 3; ++mod_count)
            {
                SyntheticGeneratorConfig config;
                zero_all_weights(config);

                if (goal.top_type == TopLevelConstruct::FunctionDef)
                    config.weights.top_level_constructs.function_def =
                        1.0;
                else if (goal.top_type == TopLevelConstruct::StructDef)
                    config.weights.top_level_constructs.struct_def =
                        1.0;
                else if (goal.top_type == TopLevelConstruct::EnumDef)
                    config.weights.top_level_constructs.enum_def =
                        1.0;
                else if (goal.top_type == TopLevelConstruct::TypeAlias)
                    config.weights.top_level_constructs.type_alias =
                        1.0;
                else if (goal.top_type == TopLevelConstruct::ImportStmt)
                    config.weights.top_level_constructs.import_stmt
                        = 1.0;
                else if (goal.top_type == TopLevelConstruct::Directive)
                    config.weights.top_level_constructs.directive =
                        1.0;
                else if (goal.top_type == TopLevelConstruct::ReturnStmt)
                    config.weights.top_level_constructs.return_stmt
                        = 1.0;
                else if (goal.top_type == TopLevelConstruct::Statement)
                    config.weights.top_level_constructs.statement =
                        1.0;

                if (goal.stmt_type.has_value())
                {
                    if (*goal.stmt_type == StatementType::SingleAssign)
                        config.weights.statement_types.single_assign =
                            1.0;
                    else if (*goal.stmt_type == StatementType::MultiAssign)
                        config.weights.statement_types.multi_assign
                            = 1.0;
                    else if (*goal.stmt_type == StatementType::Reassign) config.weights.statement_types.reassign = 1.0;
                    else if (*goal.stmt_type == StatementType::ExprStmt) config.weights.statement_types.expr_stmt = 1.0;
                }

                config.sizes.modifiers_count = {static_cast<int>(mod_count), static_cast<int>(mod_count)};

                config.registry.modifiers = 0.0;
                config.registry.statements = 0.0;
                config.registry.functions = 0.0;
                config.registry.structs = 0.0;
                config.registry.enums = 0.0;
                config.registry.type_aliases = 0.0;
                config.registry.imports = 0.0;
                config.registry.directives = 0.0;

                SyntheticGenerator gen(mod_count + static_cast<int>(goal.top_type), config);
                constexpr int requested_count = 20;
                auto [code, spec] = gen.generate_program(requested_count);
                auto ast = parse(code);

                ASSERT_NE(ast, nullptr) << "Failed to parse exclusive synthesis code for: " << goal.label;

                size_t found_target = 0;
                size_t found_others = 0;

                auto check_stmt = [&](Statement* s)
                {
                    if (goal.top_type == TopLevelConstruct::ReturnStmt)
                    {
                        if (dynamic_cast<ReturnStatement*>(s)) found_target++;
                        else found_others++;
                    }
                    else if (goal.top_type == TopLevelConstruct::Statement)
                    {
                        if (goal.stmt_type == StatementType::SingleAssign)
                        {
                            auto a = dynamic_cast<Assignment*>(s);
                            if (a && a->targets.size() == 1)
                            {
                                found_target++;
                                EXPECT_EQ(a->modifiers.size(), mod_count);
                            }
                            else found_others++;
                        }
                        else if (goal.stmt_type == StatementType::MultiAssign)
                        {
                            auto a = dynamic_cast<Assignment*>(s);
                            if (a && a->targets.size() > 1)
                            {
                                found_target++;
                                EXPECT_EQ(a->modifiers.size(), mod_count);
                            }
                            else found_others++;
                        }
                        else if (goal.stmt_type == StatementType::Reassign)
                        {
                            if (dynamic_cast<Reassignment*>(s)) found_target++;
                            else found_others++;
                        }
                        else if (goal.stmt_type == StatementType::ExprStmt)
                        {
                            if (dynamic_cast<ExpressionStatement*>(s)) found_target++;
                            else found_others++;
                        }
                    }
                    else
                    {
                        found_others++;
                    }
                };

                for (auto& f : ast->function_definitions)
                {
                    if (f->name == "ctx_wrapper")
                    {
                        for (auto& s : f->body) check_stmt(s.get());
                    }
                    else
                    {
                        if (goal.top_type == TopLevelConstruct::FunctionDef)
                        {
                            found_target++;
                            EXPECT_EQ(f->modifiers.size(), mod_count);
                        }
                        else found_others++;
                    }
                }

                if (goal.top_type == TopLevelConstruct::StructDef)
                {
                    found_target = ast->struct_definitions.size();
                    for (auto& s : ast->struct_definitions)
                        EXPECT_EQ(s->modifiers.size(), mod_count);
                }
                else found_others += ast->struct_definitions.size();

                if (goal.top_type == TopLevelConstruct::EnumDef)
                {
                    found_target = ast->enum_definitions.size();
                    for (auto& e : ast->enum_definitions)
                        EXPECT_EQ(e->modifiers.size(), mod_count);
                }
                else found_others += ast->enum_definitions.size();

                if (goal.top_type == TopLevelConstruct::TypeAlias)
                {
                    found_target = ast->type_aliases.size();
                    for (auto& a : ast->type_aliases)
                        EXPECT_EQ(a->modifiers.size(), mod_count);
                }
                else found_others += ast->type_aliases.size();

                if (goal.top_type == TopLevelConstruct::ImportStmt) found_target = ast->import_statements.size();
                else found_others += ast->import_statements.size();

                if (goal.top_type == TopLevelConstruct::Directive) found_target = ast->directives.size();
                else found_others += ast->directives.size();

                for (auto& s : ast->execution_steps) check_stmt(s.get());

                EXPECT_EQ(found_target, requested_count) << "Goal " << goal.label << " failed count check.";
                EXPECT_EQ(found_others, 0) << "Goal " << goal.label << " leaked other constructs into the AST.";
            }
        }
    }

    TEST_F(SyntheticGeneratorKnobTest, FeatureProbabilities)
    {
        const size_t iterations = get_iterations();
        SyntheticGeneratorConfig config;
        config.features.enum_case_has_value = 0.7;
        config.features.assignment_has_explicit_type = 0.2;
        config.features.func_has_docstring = 0.8;
        config.features.directive_has_value = 0.6;
        config.features.type_fallback_to_any = 0.4;
        config.features.type_is_tuple_vs_generic = 0.7;

        config.registry.types = 0.0;

        SyntheticGenerator gen(42, config);

        int enum_values = 0;
        int explicit_types = 0;
        int docstrings = 0;
        int dir_values = 0;
        int any_types = 0;
        int tuple_types = 0;
        int list_types = 0;

        for (size_t i = 0; i < iterations; ++i)
        {
            auto [e_code, e_spec] = gen.synth_enum_case();
            auto e_ast = parse("enum E: int { " + e_code + " }");
            if (e_ast && !e_ast->enum_definitions.empty() && e_ast->enum_definitions[0]->cases[0].value != nullptr)
                enum_values++;

            auto [a_code, a_spec] = gen.synth_assignment_targets(1);
            auto a_ast = parse("let " + a_code + " = 1");
            if (a_ast && !a_ast->execution_steps.empty())
            {
                if (auto assign = dynamic_cast<Assignment*>(a_ast->execution_steps[0].get()))
                {
                    if (assign->targets[0].second != nullptr) explicit_types++;
                }
            }

            auto [f_code, f_spec] = gen.logic_synth_function();
            auto f_ast = parse(f_code);
            if (f_ast && !f_ast->function_definitions.empty() && f_ast->function_definitions[0]->docstring.has_value())
                docstrings++;

            auto [d_code, d_spec] = gen.logic_synth_directive();
            auto d_ast = parse(d_code);
            if (d_ast && !d_ast->directives.empty() && d_ast->directives[0]->value != nullptr) dir_values++;

            auto [t_code, t_spec] = gen.synth_type(0);
            auto t_ast = parse("let x: " + t_code + " = 1");
            if (t_ast && !t_ast->execution_steps.empty())
            {
                auto assign = dynamic_cast<Assignment*>(t_ast->execution_steps[0].get());
                auto type_node = assign->targets[0].second.get();
                if (type_node->name == "any") any_types++;
                else if (dynamic_cast<TupleTypeAnnotation*>(type_node)) tuple_types++;
                else if (type_node->name == "List") list_types++;
            }
        }

        EXPECT_NEAR(static_cast<double>(enum_values) / iterations, 0.7, MARGIN);
        EXPECT_NEAR(static_cast<double>(explicit_types) / iterations, 0.2, MARGIN);
        EXPECT_NEAR(static_cast<double>(docstrings) / iterations, 0.8, MARGIN);
        EXPECT_NEAR(static_cast<double>(dir_values) / iterations, 0.6, MARGIN);
        EXPECT_NEAR(static_cast<double>(any_types) / iterations, 0.4, MARGIN);

        int non_any_types = static_cast<int>(iterations) - any_types;
        ASSERT_GT(non_any_types, 0);
        EXPECT_NEAR(static_cast<double>(tuple_types) / non_any_types, 0.7, MARGIN);
        EXPECT_NEAR(static_cast<double>(list_types) / non_any_types, 0.3, MARGIN);
    }

    TEST_F(SyntheticGeneratorKnobTest, RegistryProbabilities)
    {
        const size_t iterations = get_iterations();
        ASSERT_FALSE(ConstructRegistry::modifiers().empty());
        ASSERT_FALSE(ConstructRegistry::type_annotations().empty());
        ASSERT_FALSE(ConstructRegistry::expressions().empty());
        ASSERT_FALSE(ConstructRegistry::assignments().empty());

        SyntheticGeneratorConfig config;
        config.registry.modifiers = 0.3;
        config.registry.types = 0.75;
        config.registry.expressions = 0.25;
        config.registry.statements = 0.6;

        SyntheticGenerator gen(1337, config);

        std::unordered_set<std::string> reg_mods, reg_types, reg_exprs, reg_stmts;
        for (auto& item : ConstructRegistry::modifiers()) reg_mods.insert(item.code + " ");
        for (auto& item : ConstructRegistry::type_annotations()) reg_types.insert(item.code);
        for (auto& item : ConstructRegistry::expressions()) reg_exprs.insert(item.code);
        for (auto& item : ConstructRegistry::assignments()) reg_stmts.insert(item.code);
        for (auto& item : ConstructRegistry::reassignments()) reg_stmts.insert(item.code);
        for (auto& item : ConstructRegistry::expr_stmts()) reg_stmts.insert(item.code);

        int count_mods = 0, count_types = 0, count_exprs = 0, count_stmts = 0;

        for (size_t i = 0; i < iterations; ++i)
        {
            auto m_res = gen.synth_modifiers(1).first;
            if (reg_mods.contains(m_res)) count_mods++;
            EXPECT_NE(parse(m_res + "let x = 1"), nullptr);

            auto t_res = gen.synth_type(0).first;
            if (reg_types.contains(t_res)) count_types++;
            EXPECT_NE(parse("let x: " + t_res + " = 1"), nullptr);

            auto e_res = gen.synth_expression(0).first;
            if (reg_exprs.contains(e_res)) count_exprs++;
            EXPECT_NE(parse("let x = " + e_res), nullptr);

            auto s_res = gen.synth_statement().first;
            if (reg_stmts.contains(s_res)) count_stmts++;
            EXPECT_NE(parse(s_res), nullptr);
        }

        EXPECT_NEAR(static_cast<double>(count_mods) / iterations, 0.3, MARGIN);
        EXPECT_NEAR(static_cast<double>(count_types) / iterations, 0.75, MARGIN);
        EXPECT_NEAR(static_cast<double>(count_exprs) / iterations, 0.25, MARGIN);
        EXPECT_NEAR(static_cast<double>(count_stmts) / iterations, 0.6, MARGIN);
    }

    TEST_F(SyntheticGeneratorKnobTest, CategoricalWeights)
    {
        const size_t iterations = get_iterations();
        SyntheticGeneratorConfig config;

        config.weights.statement_types.single_assign = 10.0;
        config.weights.statement_types.multi_assign = 20.0;
        config.weights.statement_types.reassign = 30.0;
        config.weights.statement_types.expr_stmt = 40.0;

        config.weights.reassign_target_flavors.id = 1.0;
        config.weights.reassign_target_flavors.dot = 2.0;
        config.weights.reassign_target_flavors.bracket = 3.0;
        config.weights.reassign_target_flavors.self_dot = 4.0;

        config.weights.expression_types.binary = 1.0;
        config.weights.expression_types.dot = 4.0;
        config.weights.expression_types.bracket = 2.0;
        config.weights.expression_types.call = 2.0;
        config.weights.expression_types.grouping = 1.0;

        config.registry.statements = 0.0;
        config.registry.expressions = 0.0;

        SyntheticGenerator gen(999, config);

        int stmt_single = 0, stmt_multi = 0, stmt_reassign = 0, stmt_expr = 0;
        int re_id = 0, re_dot = 0, re_bracket = 0, re_self = 0;
        int exp_bin = 0, exp_dot = 0, exp_bracket = 0, exp_call = 0, exp_group = 0;

        for (size_t i = 0; i < iterations; ++i)
        {
            auto s_code = gen.logic_synth_statement().first;
            auto s_ast = parse(s_code);
            if (s_ast && !s_ast->execution_steps.empty())
            {
                auto stmt = s_ast->execution_steps[0].get();
                if (auto assign = dynamic_cast<Assignment*>(stmt))
                {
                    if (assign->targets.size() == 1) stmt_single++;
                    else stmt_multi++;
                }
                else if (dynamic_cast<ExpressionStatement*>(stmt))
                {
                    stmt_expr++;
                }
                else if (auto re = dynamic_cast<Reassignment*>(stmt))
                {
                    stmt_reassign++;
                    auto tgt = re->target.get();
                    if (dynamic_cast<IdentifierAccess*>(tgt)) re_id++;
                    else if (auto dot = dynamic_cast<DotAccess*>(tgt))
                    {
                        if (dynamic_cast<SelfExpression*>(dot->target.get())) re_self++;
                        else re_dot++;
                    }
                    else if (dynamic_cast<BracketAccess*>(tgt))
                    {
                        re_bracket++;
                    }
                }
            }

            auto e_code = gen.synth_expression(2, 3).first;
            auto e_ast = parse("let x = " + e_code);
            if (e_ast && !e_ast->execution_steps.empty())
            {
                auto expr = dynamic_cast<Assignment*>(e_ast->execution_steps[0].get())->value.get();
                expr = unwrap_grouping(expr);
                if (dynamic_cast<BinaryExpression*>(expr)) exp_bin++;
                else if (dynamic_cast<DotAccess*>(expr)) exp_dot++;
                else if (dynamic_cast<BracketAccess*>(expr)) exp_bracket++;
                else if (dynamic_cast<FunctionCall*>(expr)) exp_call++;
                else exp_group++;
            }
        }

        double total_stmt = stmt_single + stmt_multi + stmt_reassign + stmt_expr;
        ASSERT_GT(total_stmt, 0);
        EXPECT_NEAR(stmt_single / total_stmt, 0.1, MARGIN);
        EXPECT_NEAR(stmt_multi / total_stmt, 0.2, MARGIN);
        EXPECT_NEAR(stmt_reassign / total_stmt, 0.3, MARGIN);
        EXPECT_NEAR(stmt_expr / total_stmt, 0.4, MARGIN);

        double total_re = re_id + re_dot + re_bracket + re_self;
        ASSERT_GT(total_re, 0);
        EXPECT_NEAR(re_id / total_re, 0.1, MARGIN);
        EXPECT_NEAR(re_dot / total_re, 0.2, MARGIN);
        EXPECT_NEAR(re_bracket / total_re, 0.3, MARGIN);
        EXPECT_NEAR(re_self / total_re, 0.4, MARGIN);

        double total_exp = exp_bin + exp_dot + exp_bracket + exp_call + exp_group;
        ASSERT_GT(total_exp, 0);
        EXPECT_NEAR(exp_bin / total_exp, 0.1, MARGIN);
        EXPECT_NEAR(exp_dot / total_exp, 0.4, MARGIN);
        EXPECT_NEAR(exp_bracket / total_exp, 0.2, MARGIN);
        EXPECT_NEAR(exp_call / total_exp, 0.2, MARGIN);
        EXPECT_NEAR(exp_group / total_exp, 0.1, MARGIN);
    }

    TEST_F(SyntheticGeneratorKnobTest, HarvestStatementWeights)
    {
        const size_t iterations = get_iterations();
        SyntheticGeneratorConfig config;

        config.weights.harvest_statement_types.assignment = 5.0;
        config.weights.harvest_statement_types.reassignment = 3.0;
        config.weights.harvest_statement_types.expr_stmt = 2.0;

        config.registry.statements = 1.0;

        SyntheticGenerator gen(111, config);

        int count_assign = 0, count_reassign = 0, count_expr = 0;
        for (size_t i = 0; i < iterations; ++i)
        {
            auto s_code = gen.harvest_statement().first;
            auto s_ast = parse(s_code);
            if (s_ast && !s_ast->execution_steps.empty())
            {
                auto stmt = s_ast->execution_steps[0].get();
                if (dynamic_cast<Assignment*>(stmt)) count_assign++;
                else if (dynamic_cast<Reassignment*>(stmt)) count_reassign++;
                else if (dynamic_cast<ExpressionStatement*>(stmt)) count_expr++;
            }
        }

        auto total = static_cast<double>(count_assign + count_reassign + count_expr);
        EXPECT_NEAR(count_assign / total, 0.5, MARGIN);
        EXPECT_NEAR(count_reassign / total, 0.3, MARGIN);
        EXPECT_NEAR(count_expr / total, 0.2, MARGIN);
    }

    TEST_F(SyntheticGeneratorKnobTest, TopLevelConstructWeights)
    {
        const size_t iterations = get_iterations();
        SyntheticGeneratorConfig config;

        config.weights.top_level_constructs.expression = 1.0;
        config.weights.top_level_constructs.type_annotation = 2.0;
        config.weights.top_level_constructs.statement = 3.0;
        config.weights.top_level_constructs.return_stmt = 4.0;
        config.weights.top_level_constructs.modifier = 5.0;
        config.weights.top_level_constructs.function_def = 6.0;
        config.weights.top_level_constructs.struct_def = 7.0;
        config.weights.top_level_constructs.enum_def = 8.0;
        config.weights.top_level_constructs.type_alias = 9.0;
        config.weights.top_level_constructs.import_stmt = 10.0;
        config.weights.top_level_constructs.directive = 11.0;

        SyntheticGenerator gen(555, config);

        std::vector<int> counts(static_cast<int>(TopLevelConstruct::Count), 0);
        for (size_t i = 0; i < iterations; ++i)
        {
            TopLevelConstruct choice = gen.roll_top_level_construct();
            counts[static_cast<int>(choice)]++;
        }

        auto total = static_cast<double>(iterations);
        double weight_sum = 66.0;

        EXPECT_NEAR(counts[static_cast<int>(TopLevelConstruct::Expression)] / total, 1.0 / weight_sum, MARGIN);
        EXPECT_NEAR(counts[static_cast<int>(TopLevelConstruct::TypeAnnotation)] / total, 2.0 / weight_sum, MARGIN);
        EXPECT_NEAR(counts[static_cast<int>(TopLevelConstruct::Statement)] / total, 3.0 / weight_sum, MARGIN);
        EXPECT_NEAR(counts[static_cast<int>(TopLevelConstruct::ReturnStmt)] / total, 4.0 / weight_sum, MARGIN);
        EXPECT_NEAR(counts[static_cast<int>(TopLevelConstruct::Modifier)] / total, 5.0 / weight_sum, MARGIN);
        EXPECT_NEAR(counts[static_cast<int>(TopLevelConstruct::FunctionDef)] / total, 6.0 / weight_sum, MARGIN);
        EXPECT_NEAR(counts[static_cast<int>(TopLevelConstruct::StructDef)] / total, 7.0 / weight_sum, MARGIN);
        EXPECT_NEAR(counts[static_cast<int>(TopLevelConstruct::EnumDef)] / total, 8.0 / weight_sum, MARGIN);
        EXPECT_NEAR(counts[static_cast<int>(TopLevelConstruct::TypeAlias)] / total, 9.0 / weight_sum, MARGIN);
        EXPECT_NEAR(counts[static_cast<int>(TopLevelConstruct::ImportStmt)] / total, 10.0 / weight_sum, MARGIN);
        EXPECT_NEAR(counts[static_cast<int>(TopLevelConstruct::Directive)] / total, 11.0 / weight_sum, MARGIN);
    }

    TEST_F(SyntheticGeneratorKnobTest, SizeRanges)
    {
        const size_t iterations = get_iterations();
        SyntheticGeneratorConfig config;

        config.sizes.modifiers_count = {2, 4};
        config.sizes.modifier_arguments = {1, 3};
        config.sizes.function_statements = {2, 5};
        config.sizes.function_parameters = {1, 4};
        config.sizes.struct_fields = {2, 5};
        config.sizes.enum_cases = {3, 6};
        config.sizes.multi_assign_targets = {3, 6};

        config.registry.modifiers = 0.0;
        config.registry.types = 0.0;
        config.registry.expressions = 0.0;
        config.registry.statements = 0.0;
        config.registry.functions = 0.0;
        config.registry.structs = 0.0;
        config.registry.enums = 0.0;
        config.registry.type_aliases = 0.0;
        config.registry.imports = 0.0;
        config.registry.directives = 0.0;

        SyntheticGenerator gen(222, config);

        size_t test_size = std::min(iterations, static_cast<size_t>(1000));
        for (size_t i = 0; i < test_size; ++i)
        {
            auto [m_code, m_specs] = gen.synth_modifiers(gen.rand_range(config.sizes.modifiers_count));
            auto m_ast = parse(m_code + "let x = 1");
            if (m_ast && !m_ast->execution_steps.empty())
            {
                auto assign = dynamic_cast<Assignment*>(m_ast->execution_steps[0].get());
                if (assign)
                {
                    auto mods = &assign->modifiers;
                    EXPECT_GE(mods->size(), 2);
                    EXPECT_LE(mods->size(), 4);
                    for (auto& mod : *mods)
                    {
                        EXPECT_GE(mod.arguments.size(), 1);
                        EXPECT_LE(mod.arguments.size(), 3);
                    }
                }
            }

            auto [t_code, t_specs] = gen.synth_assignment_targets(gen.rand_range(config.sizes.multi_assign_targets));
            auto t_ast = parse("let " + t_code + " = 1");
            if (t_ast && !t_ast->execution_steps.empty())
            {
                if (auto assign = dynamic_cast<Assignment*>(t_ast->execution_steps[0].get()))
                {
                    EXPECT_GE(assign->targets.size(), 3);
                    EXPECT_LE(assign->targets.size(), 6);
                }
            }

            auto [f_code, f_spec] = gen.logic_synth_function();
            auto f_ast = parse(f_code);
            if (f_ast && !f_ast->function_definitions.empty())
            {
                EXPECT_GE(f_ast->function_definitions[0]->parameters.size(), 1);
                EXPECT_LE(f_ast->function_definitions[0]->parameters.size(), 4);
                EXPECT_GE(f_ast->function_definitions[0]->body.size(), 2);
                EXPECT_LE(f_ast->function_definitions[0]->body.size(), 5);
            }

            auto [s_code, s_spec] = gen.logic_synth_struct();
            auto s_ast = parse(s_code);
            if (s_ast && !s_ast->struct_definitions.empty())
            {
                EXPECT_GE(s_ast->struct_definitions[0]->fields.size(), 2);
                EXPECT_LE(s_ast->struct_definitions[0]->fields.size(), 5);
            }

            auto [e_code, e_spec] = gen.logic_synth_enum();
            auto e_ast = parse(e_code);
            if (e_ast && !e_ast->enum_definitions.empty())
            {
                EXPECT_GE(e_ast->enum_definitions[0]->cases.size(), 3);
                EXPECT_LE(e_ast->enum_definitions[0]->cases.size(), 6);
            }

            auto [a_code, a_spec] = gen.logic_synth_type_alias();
            auto a_ast = parse(a_code);
            if (a_ast && !a_ast->type_aliases.empty())
            {
                EXPECT_GE(a_ast->type_aliases[0]->modifiers.size(), 2);
                EXPECT_LE(a_ast->type_aliases[0]->modifiers.size(), 4);
            }
        }
    }

    TEST_F(SyntheticGeneratorKnobTest, AllKnobsToZeroEmptyAST)
    {
        SyntheticGeneratorConfig config;
        zero_all_weights(config);

        for (size_t seed = 0; seed < 100; ++seed)
        {
            SyntheticGenerator gen(seed, config);

            auto [code, spec] = gen.generate_program(100);

            EXPECT_EQ(code, "") << "Generator produced code even though all top-level weights were 0 at seed " << seed;

            EXPECT_TRUE(spec.imports.empty());
            EXPECT_TRUE(spec.directives.empty());
            EXPECT_TRUE(spec.execution_steps.empty());
            EXPECT_TRUE(spec.functions.empty());
            EXPECT_TRUE(spec.structs.empty());
            EXPECT_TRUE(spec.enums.empty());
            EXPECT_TRUE(spec.type_aliases.empty());
        }
    }
}
