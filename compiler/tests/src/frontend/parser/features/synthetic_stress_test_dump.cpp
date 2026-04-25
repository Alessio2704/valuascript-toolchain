#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/synthetic_generator.h"
#include "frontend/parser/helpers/dump_writer.h"
#include <fstream>
#include <iostream>

namespace valuascript::compiler::test
{
    class SyntheticStressTest : public ParserTestBase
    {
    };

    TEST_F(SyntheticStressTest, InspectGeneratedPrograms)
    {
        std::filesystem::path dump_dir = std::filesystem::current_path() / "fuzz_dumps";
        std::filesystem::create_directories(dump_dir);

        std::cout << "\n======================================================\n";
        std::cout << "Dumping synthetic programs to: \n" << dump_dir.string() << "\n";
        std::cout << "======================================================\n\n";

        for (size_t seed = 0; seed < 5; ++seed)
        {
            SyntheticGenerator gen(seed);
            auto [code, spec] = gen.generate_program(1000);

            DumpWriter writer("program_seed_" + std::to_string(seed) + ".vs");

            ASSERT_TRUE(writer.out().is_open()) << "Failed to open file for writing: " << writer.path_string();

            writer.out() << "// ==========================================\n";
            writer.out() << "// Synthetic Program Dump (Seed: " << seed << ")\n";
            writer.out() << "// ==========================================\n\n";
            writer.out() << code;
        }
    }

    TEST_F(SyntheticStressTest, InspectGeneratedProgramsWithConfig)
    {
        SyntheticGeneratorConfig cfg;

        cfg.registry.modifiers = 0.0;
        cfg.registry.types = 0.0;
        cfg.registry.returns = 1.0;
        cfg.registry.expressions = 0.0;
        cfg.registry.statements = 0.0;
        cfg.registry.functions = 0.0;
        cfg.registry.structs = 0.0;
        cfg.registry.enums = 0.0;
        cfg.registry.type_aliases = 0.0;
        cfg.registry.imports = 0.0;
        cfg.registry.directives = 0.0;

        cfg.features.enum_case_has_value = 0.0;
        cfg.features.assignment_has_explicit_type = 0.0;
        cfg.features.func_has_docstring = 0.0;
        cfg.features.directive_has_value = 0.0;
        cfg.features.type_fallback_to_any = 0.0;
        cfg.features.type_is_tuple_vs_generic = 0.0;

        cfg.sizes.modifiers_count = {0, 0};
        cfg.sizes.standalone_modifiers_count = {0, 0};
        cfg.sizes.modifier_arguments = {0, 0};
        cfg.sizes.function_statements = {0, 0};
        cfg.sizes.function_parameters = {0, 0};
        cfg.sizes.struct_fields = {0, 0};
        cfg.sizes.enum_cases = {0, 0};
        cfg.sizes.multi_assign_targets = {0, 0};

        cfg.weights.top_level_constructs.expression = 0.0;
        cfg.weights.top_level_constructs.type_annotation = 0.0;
        cfg.weights.top_level_constructs.statement = 0.0;
        cfg.weights.top_level_constructs.return_stmt = 0.0;
        cfg.weights.top_level_constructs.modifier = 0.0;
        cfg.weights.top_level_constructs.function_def = 0.0;
        cfg.weights.top_level_constructs.struct_def = 0.0;
        cfg.weights.top_level_constructs.enum_def = 0.0;
        cfg.weights.top_level_constructs.type_alias = 0.0;
        cfg.weights.top_level_constructs.import_stmt = 0.0;
        cfg.weights.top_level_constructs.directive = 0.0;

        cfg.weights.statement_types.single_assign = 0.0;
        cfg.weights.statement_types.multi_assign = 0.0;
        cfg.weights.statement_types.reassign = 0.0;
        cfg.weights.statement_types.expr_stmt = 0.0;

        cfg.weights.reassign_target_flavors.id = 0.0;
        cfg.weights.reassign_target_flavors.dot = 0.0;
        cfg.weights.reassign_target_flavors.bracket = 0.0;
        cfg.weights.reassign_target_flavors.self_dot = 0.0;

        cfg.weights.expression_types.binary = 0.0;
        cfg.weights.expression_types.dot = 0.0;
        cfg.weights.expression_types.bracket = 0.0;
        cfg.weights.expression_types.call = 0.0;
        cfg.weights.expression_types.grouping = 0.0;

        cfg.weights.harvest_statement_types.assignment = 0.0;
        cfg.weights.harvest_statement_types.reassignment = 0.0;
        cfg.weights.harvest_statement_types.expr_stmt = 0.0;

        std::filesystem::path dump_dir = std::filesystem::current_path() / "fuzz_dumps";
        std::filesystem::create_directories(dump_dir);

        for (size_t seed = 0; seed < 5; ++seed)
        {
            SyntheticGenerator gen(seed, cfg);
            auto [code, spec] = gen.generate_program(1000);

            std::filesystem::path file_path = dump_dir / ("experiment_seed_" + std::to_string(seed) + ".vs");
            std::ofstream out(file_path);

            ASSERT_TRUE(out.is_open()) << "Failed to open file for writing: " << file_path;

            out << cfg.generate_report(seed);
            out << code;
            out.close();
        }
    }
}
