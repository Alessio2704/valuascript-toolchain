#include <gtest/gtest.h>
#include <unordered_set>
#include <string>
#include <functional>
#include "utils/test_env_config.h"
#include "synthetic_generator.h"
#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class SyntheticGeneratorKnobTest : public ParserTestBase
    {
    protected:
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

        static SyntheticGeneratorConfig strict_config()
        {
            SyntheticGeneratorConfig c;
            c.registry.modifiers = 0.0;
            c.registry.types = 0.0;
            c.registry.expressions = 0.0;
            c.registry.returns = 0.0;
            c.registry.statements = 0.0;
            c.registry.functions = 0.0;
            c.registry.structs = 0.0;
            c.registry.enums = 0.0;
            c.registry.type_aliases = 0.0;
            c.registry.imports = 0.0;
            c.registry.directives = 0.0;

            c.sizes.modifiers_count = {0, 0};
            c.sizes.standalone_modifiers_count = {0, 0};
            c.features.assignment_has_let_modifiers = false;

            return c;
        }
    };

    TEST_F(SyntheticGeneratorKnobTest, ExhaustiveGrammarValidation)
    {
        SyntheticGeneratorConfig config = strict_config();

        config.sizes.modifiers_count = {2, 4};
        config.sizes.standalone_modifiers_count = {2, 4};
        config.sizes.modifier_arguments = {1, 3};
        config.sizes.function_statements = {2, 5};
        config.sizes.function_parameters = {1, 4};
        config.sizes.struct_fields = {2, 5};
        config.sizes.enum_cases = {3, 6};
        config.sizes.multi_assign_targets = {3, 6};
        config.sizes.switch_cases = {2, 4};
        config.sizes.dict_elements = {2, 5};
        config.sizes.tuple_elements = {3, 5};
        config.sizes.tensor_elements = {2, 4};
        config.features.assignment_has_let_modifiers = true;

        SyntheticGenerator gen(42, config);

        auto test_rule_runner = [&]<typename VerifierT>(const GrammarRule<VerifierT>& rule)
        {
            for (int i = 0; i < 20; ++i)
            {
                auto [code, verifier] = rule.rule(gen, 0);
                std::string full_code = rule.test_prefix + code + rule.test_suffix;

                auto ast = parse(full_code);
                ASSERT_NE(ast, nullptr) << "Parse failed for rule: " << rule.name << "\nCode: " << full_code;

                rule.validate(ast.get(), verifier);
            }
        };

        gen.for_each_rule(test_rule_runner);
    }

    TEST_F(SyntheticGeneratorKnobTest, FeatureProbabilities)
    {
        constexpr size_t iterations = FUZZ_ITERATIONS;

        {
            SyntheticGeneratorConfig c = strict_config();
            c.features.enum_case_has_value = 0.7;
            SyntheticGenerator gen(1, c);
            int hits = 0;
            for (size_t i = 0; i < iterations; ++i)
            {
                auto [code, spec] = gen.generate_raw_enum_case(0);
                if (spec.value_v != nullptr) hits++;
            }
            EXPECT_NEAR(static_cast<double>(hits) / iterations, 0.7, MARGIN);
        }

        {
            SyntheticGeneratorConfig c = strict_config();
            c.features.assignment_has_explicit_type = 0.2;
            c.sizes.multi_assign_targets = {1, 1};
            SyntheticGenerator gen(2, c);
            int hits = 0;
            for (size_t i = 0; i < iterations; ++i)
            {
                auto [code, specs] = gen.generate_raw_assignment_targets(0);
                if (!specs.empty() && specs[0].type_v != nullptr) hits++;
            }
            EXPECT_NEAR(static_cast<double>(hits) / iterations, 0.2, MARGIN);
        }

        {
            SyntheticGeneratorConfig c = strict_config();
            c.features.func_has_docstring = 0.8;
            SyntheticGenerator gen(3, c);
            int hits = 0;
            for (size_t i = 0; i < iterations; ++i)
            {
                auto code = gen.generate_raw_function(0).first;
                if (code.find("\"\"\"") != std::string::npos) hits++;
            }
            EXPECT_NEAR(static_cast<double>(hits) / iterations, 0.8, MARGIN);
        }

        {
            SyntheticGeneratorConfig c = strict_config();
            c.features.directive_has_value = 0.6;
            SyntheticGenerator gen(4, c);
            int hits = 0;
            for (size_t i = 0; i < iterations; ++i)
            {
                auto code = gen.generate_raw_directive(0).first;
                if (code.find("=") != std::string::npos) hits++;
            }
            EXPECT_NEAR(static_cast<double>(hits) / iterations, 0.6, MARGIN);
        }

        {
            SyntheticGeneratorConfig c = strict_config();
            c.features.type_fallback_to_any = 0.4;
            c.features.type_is_tuple_vs_generic = 0.7;
            SyntheticGenerator gen(5, c);

            int any_types = 0, tuple_types = 0, list_types = 0;
            for (size_t i = 0; i < iterations; ++i)
            {
                auto code = gen.generate_raw_type(0).first;
                if (code == "any") any_types++;
                else if (!code.empty() && code.front() == '(') tuple_types++;
                else if (code.find("List") == 0) list_types++;
            }

            EXPECT_NEAR(static_cast<double>(any_types) / iterations, 0.4, MARGIN);

            int non_any_types = static_cast<int>(iterations) - any_types;
            ASSERT_GT(non_any_types, 0);
            EXPECT_NEAR(static_cast<double>(tuple_types) / non_any_types, 0.7, MARGIN);
            EXPECT_NEAR(static_cast<double>(list_types) / non_any_types, 0.3, MARGIN);
        }
    }

    TEST_F(SyntheticGeneratorKnobTest, RegistryProbabilities)
    {
        constexpr size_t iterations = FUZZ_ITERATIONS;
        ASSERT_FALSE(ConstructRegistry::modifiers().empty());
        ASSERT_FALSE(ConstructRegistry::type_annotations().empty());
        ASSERT_FALSE(ConstructRegistry::expressions().empty());
        ASSERT_FALSE(ConstructRegistry::assignments().empty());

        auto check_registry_hit = [](const std::string& code, const auto& registry)
        {
            std::string trimmed = code;
            while (!trimmed.empty() && trimmed.back() == ' ') trimmed.pop_back();

            for (const auto& item : registry)
            {
                std::string item_trimmed = item.code;
                while (!item_trimmed.empty() && item_trimmed.back() == ' ') item_trimmed.pop_back();

                if (trimmed == item_trimmed) return true;
            }
            return false;
        };

        {
            SyntheticGeneratorConfig c = strict_config();
            c.registry.modifiers = 0.3;
            c.sizes.modifiers_count = {1, 1};
            SyntheticGenerator gen(101, c);
            int hits = 0;
            for (size_t i = 0; i < iterations; ++i)
            {
                if (check_registry_hit(gen.generate_raw_modifiers(0).first, ConstructRegistry::modifiers())) hits++;
            }
            EXPECT_NEAR(static_cast<double>(hits) / iterations, 0.3, MARGIN);
        }

        {
            SyntheticGeneratorConfig c = strict_config();
            c.registry.types = 0.75;
            SyntheticGenerator gen(102, c);
            int hits = 0;
            for (size_t i = 0; i < iterations; ++i)
            {
                if (check_registry_hit(gen.generate_raw_type(0).first, ConstructRegistry::type_annotations())) hits++;
            }
            EXPECT_NEAR(static_cast<double>(hits) / iterations, 0.75, MARGIN);
        }

        {
            SyntheticGeneratorConfig c = strict_config();
            c.registry.expressions = 0.25;
            SyntheticGenerator gen(103, c);
            int hits = 0;
            for (size_t i = 0; i < iterations; ++i)
            {
                if (check_registry_hit(gen.generate_raw_expression(0).first, ConstructRegistry::expressions())) hits++;
            }
            EXPECT_NEAR(static_cast<double>(hits) / iterations, 0.25, MARGIN);
        }

        {
            SyntheticGeneratorConfig c = strict_config();
            c.registry.statements = 0.6;
            SyntheticGenerator gen(104, c);
            int hits = 0;
            for (size_t i = 0; i < iterations; ++i)
            {
                auto code = gen.generate_raw_statement(0).first;
                if (check_registry_hit(code, ConstructRegistry::assignments()) ||
                    check_registry_hit(code, ConstructRegistry::reassignments()) ||
                    check_registry_hit(code, ConstructRegistry::expr_stmts()))
                {
                    hits++;
                }
            }
            EXPECT_NEAR(static_cast<double>(hits) / iterations, 0.6, MARGIN);
        }
    }

    TEST_F(SyntheticGeneratorKnobTest, CategoricalWeights)
    {
        constexpr size_t iterations = FUZZ_ITERATIONS;

        {
            SyntheticGeneratorConfig config = strict_config();

            config.weights.statement_types.single_assign = 10.0;
            config.weights.statement_types.multi_assign = 20.0;
            config.weights.statement_types.reassign = 30.0;
            config.weights.statement_types.expr_stmt = 40.0;

            config.weights.reassign_target_flavors.id = 1.0;
            config.weights.reassign_target_flavors.dot = 2.0;
            config.weights.reassign_target_flavors.bracket = 3.0;
            config.weights.reassign_target_flavors.self_dot = 4.0;

            SyntheticGenerator gen(999, config);

            int stmt_single = 0, stmt_multi = 0, stmt_reassign = 0, stmt_expr = 0;
            int re_id = 0, re_dot = 0, re_bracket = 0, re_self = 0;

            for (size_t i = 0; i < iterations; ++i)
            {
                auto code = gen.generate_raw_statement(0).first;
                auto ast = parse(code);

                if (ast && !ast->execution_steps.empty())
                {
                    Statement* stmt = ast->execution_steps[0].get();
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
                        else if (dynamic_cast<BracketAccess*>(tgt)) re_bracket++;
                    }
                }
            }

            double total_stmt = stmt_single + stmt_multi + stmt_reassign + stmt_expr;
            ASSERT_GT(total_stmt, 0);
            EXPECT_NEAR(stmt_single / total_stmt, 0.1, MARGIN + 0.02);
            EXPECT_NEAR(stmt_multi / total_stmt, 0.2, MARGIN + 0.02);
            EXPECT_NEAR(stmt_reassign / total_stmt, 0.3, MARGIN + 0.02);
            EXPECT_NEAR(stmt_expr / total_stmt, 0.4, MARGIN + 0.02);

            double total_re = re_id + re_dot + re_bracket + re_self;
            ASSERT_GT(total_re, 0);
            EXPECT_NEAR(re_id / total_re, 0.1, MARGIN + 0.03);
            EXPECT_NEAR(re_dot / total_re, 0.2, MARGIN + 0.03);
            EXPECT_NEAR(re_bracket / total_re, 0.3, MARGIN + 0.03);
            EXPECT_NEAR(re_self / total_re, 0.4, MARGIN + 0.03);
        }

        {
            SyntheticGeneratorConfig config = strict_config();
            config.weights.expression_types.binary = 2.0;
            config.weights.expression_types.unary = 1.0;
            config.weights.expression_types.dot = 2.0;
            config.weights.expression_types.bracket = 2.0;
            config.weights.expression_types.call = 2.0;
            config.weights.expression_types.grouping = 1.0;
            config.weights.expression_types.switch_expr = 2.0;
            config.weights.expression_types.dict_expr = 2.0;
            config.weights.expression_types.tuple_expr = 2.0;
            config.weights.expression_types.tensor_expr = 2.0;
            config.weights.expression_types.conditional = 2.0;

            SyntheticGenerator gen(1000, config);

            int exp_bin = 0, exp_un = 0, exp_dot = 0, exp_bracket = 0, exp_call = 0;
            int exp_switch = 0, exp_dict = 0, exp_tuple = 0, exp_tensor = 0, exp_cond = 0, exp_group = 0;

            for (size_t i = 0; i < iterations; ++i)
            {
                auto code = gen.generate_raw_expression(0).first;

                auto ast = parse("let x = " + code);
                if (ast && !ast->execution_steps.empty())
                {
                    if (auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get()))
                    {
                        Expression* expr = assign->value.get();

                        if (dynamic_cast<BinaryExpression*>(expr)) exp_bin++;
                        else if (dynamic_cast<UnaryExpression*>(expr)) exp_un++;
                        else if (dynamic_cast<DotAccess*>(expr)) exp_dot++;
                        else if (dynamic_cast<BracketAccess*>(expr)) exp_bracket++;
                        else if (dynamic_cast<FunctionCall*>(expr)) exp_call++;
                        else if (dynamic_cast<SwitchExpression*>(expr)) exp_switch++;
                        else if (dynamic_cast<DictLiteral*>(expr)) exp_dict++;
                        else if (dynamic_cast<TupleLiteral*>(expr)) exp_tuple++;
                        else if (dynamic_cast<TensorLiteral*>(expr)) exp_tensor++;
                        else if (dynamic_cast<ConditionalExpression*>(expr)) exp_cond++;
                        else if (dynamic_cast<GroupingExpression*>(expr)) exp_group++;
                    }
                }
            }

            double total_exp = exp_bin + exp_un + exp_dot + exp_bracket + exp_call +
                exp_switch + exp_dict + exp_tuple + exp_tensor + exp_cond + exp_group;

            ASSERT_GT(total_exp, 0);
            EXPECT_NEAR(exp_bin / total_exp, 0.10, MARGIN + 0.02);
            EXPECT_NEAR(exp_un / total_exp, 0.05, MARGIN + 0.02);
            EXPECT_NEAR(exp_dot / total_exp, 0.10, MARGIN + 0.02);
            EXPECT_NEAR(exp_bracket / total_exp, 0.10, MARGIN + 0.02);
            EXPECT_NEAR(exp_call / total_exp, 0.10, MARGIN + 0.02);
            EXPECT_NEAR(exp_switch / total_exp, 0.10, MARGIN + 0.02);
            EXPECT_NEAR(exp_dict / total_exp, 0.10, MARGIN + 0.02);
            EXPECT_NEAR(exp_tuple / total_exp, 0.10, MARGIN + 0.02);
            EXPECT_NEAR(exp_tensor / total_exp, 0.10, MARGIN + 0.02);
            EXPECT_NEAR(exp_cond / total_exp, 0.10, MARGIN + 0.02);
            EXPECT_NEAR(exp_group / total_exp, 0.05, MARGIN + 0.02);
        }
    }

    TEST_F(SyntheticGeneratorKnobTest, HarvestStatementWeights)
    {
        constexpr size_t iterations = FUZZ_ITERATIONS;
        SyntheticGeneratorConfig config = strict_config();

        config.weights.harvest_statement_types.assignment = 5.0;
        config.weights.harvest_statement_types.reassignment = 3.0;
        config.weights.harvest_statement_types.expr_stmt = 2.0;

        config.registry.statements = 1.0;

        SyntheticGenerator gen(111, config);

        int count_assign = 0, count_reassign = 0, count_expr = 0;
        for (size_t i = 0; i < iterations; ++i)
        {
            auto code = gen.generate_raw_statement(0).first;
            auto ast = parse(code);

            if (ast && !ast->execution_steps.empty())
            {
                Statement* stmt = ast->execution_steps[0].get();
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
        constexpr size_t iterations = FUZZ_ITERATIONS;
        SyntheticGeneratorConfig config = strict_config();

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
            counts[static_cast<size_t>(choice)]++;
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
        constexpr size_t iterations = FUZZ_ITERATIONS;
        SyntheticGeneratorConfig config = strict_config();

        config.sizes.modifiers_count = {2, 4};
        config.sizes.standalone_modifiers_count = {2, 4};
        config.sizes.modifier_arguments = {1, 3};
        config.sizes.function_statements = {2, 5};
        config.sizes.function_parameters = {1, 4};
        config.sizes.struct_fields = {2, 5};
        config.sizes.enum_cases = {3, 6};
        config.sizes.multi_assign_targets = {3, 6};
        config.sizes.switch_cases = {2, 4};
        config.sizes.dict_elements = {2, 5};
        config.sizes.tuple_elements = {3, 5};
        config.sizes.tensor_elements = {2, 4};

        SyntheticGenerator gen(222, config);

        size_t test_size = std::min(iterations, static_cast<size_t>(1000));

        for (size_t i = 0; i < test_size; ++i)
        {
            auto [m_code, m_specs] = gen.generate_raw_modifiers(0);
            EXPECT_GE(m_specs.size(), 2);
            EXPECT_LE(m_specs.size(), 4);
            for (const auto& mod : m_specs)
            {
                EXPECT_GE(mod.args.size(), 1);
                EXPECT_LE(mod.args.size(), 3);
            }

            auto [t_code, t_specs] = gen.generate_raw_assignment_targets(0);
            EXPECT_GE(t_specs.size(), 3);
            EXPECT_LE(t_specs.size(), 6);

            auto f_code = gen.generate_raw_function(0).first;
            auto f_ast = parse(f_code);
            if (f_ast && !f_ast->function_definitions.empty())
            {
                auto f = f_ast->function_definitions[0].get();
                EXPECT_GE(f->parameters.size(), 1);
                EXPECT_LE(f->parameters.size(), 4);
                EXPECT_GE(f->body.size(), 2);
                EXPECT_LE(f->body.size(), 5);
            }

            auto s_code = gen.generate_raw_struct(0).first;
            auto s_ast = parse(s_code);
            if (s_ast && !s_ast->struct_definitions.empty())
            {
                EXPECT_GE(s_ast->struct_definitions[0]->fields.size(), 2);
                EXPECT_LE(s_ast->struct_definitions[0]->fields.size(), 5);
            }

            auto e_code = gen.generate_raw_enum(0).first;
            auto e_ast = parse(e_code);
            if (e_ast && !e_ast->enum_definitions.empty())
            {
                EXPECT_GE(e_ast->enum_definitions[0]->cases.size(), 3);
                EXPECT_LE(e_ast->enum_definitions[0]->cases.size(), 6);
            }

            auto expr_code = gen.generate_raw_expression(0).first;
            auto expr_ast = parse("let x = " + expr_code);
            if (expr_ast && !expr_ast->execution_steps.empty())
            {
                if (auto assign = dynamic_cast<Assignment*>(expr_ast->execution_steps[0].get()))
                {
                    Expression* val = assign->value.get();
                    if (auto sw = dynamic_cast<SwitchExpression*>(val))
                    {
                        EXPECT_GE(sw->cases.size(), 2);
                        EXPECT_LE(sw->cases.size(), 4);
                    }
                    else if (auto dict = dynamic_cast<DictLiteral*>(val))
                    {
                        EXPECT_GE(dict->elements.size(), 2);
                        EXPECT_LE(dict->elements.size(), 5);
                    }
                    else if (auto tup = dynamic_cast<TupleLiteral*>(val))
                    {
                        EXPECT_GE(tup->elements.size(), 3);
                        EXPECT_LE(tup->elements.size(), 5);
                    }
                    else if (auto ten = dynamic_cast<TensorLiteral*>(val))
                    {
                        EXPECT_GE(ten->elements.size(), 2);
                        EXPECT_LE(ten->elements.size(), 4);
                    }
                }
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
