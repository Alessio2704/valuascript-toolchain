#include <gtest/gtest.h>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <iomanip>
#include <algorithm>

#include "testing_framework_helpers.h"
#include "frontend/parser/helpers/dump_writer.h"
#include "frontend/parser/helpers/construct_registry.h"
#include "frontend/parser/helpers/error_registry.h"
#include "frontend/parser/helpers/context_registry.h"
#include "frontend/parser/helpers/test_structures.h"
#include "frontend/parser/expansion_and_sentinels/expansion_calculator.h"
#include "frontend/parser/expansion_and_sentinels/expansion_policy.h"

namespace valuascript::compiler::test
{
    static std::pair<size_t, size_t> get_registry_counts(InjectableType type)
    {
        switch (type)
        {
        case InjectableType::Identifier:
            break;
        case InjectableType::Import:
            return {ConstructRegistry::imports().size(), ErrorRegistry::imports().size()};
        case InjectableType::Directive:
            return {ConstructRegistry::directives().size(), ErrorRegistry::directives().size()};
        case InjectableType::Function:
            return {ConstructRegistry::functions().size(), ErrorRegistry::functions().size()};
        case InjectableType::Extension:
            return {ConstructRegistry::extensions().size(), ErrorRegistry::extensions().size()};
        case InjectableType::Struct:
            return {ConstructRegistry::structs().size(), ErrorRegistry::structs().size()};
        case InjectableType::Enum:
            return {ConstructRegistry::enums().size(), ErrorRegistry::enums().size()};
        case InjectableType::TypeAlias:
            return {ConstructRegistry::aliases().size(), ErrorRegistry::aliases().size()};
        case InjectableType::Expression:
            return {ConstructRegistry::expressions().size(), ErrorRegistry::expressions().size()};
        case InjectableType::Modifier:
            return {ConstructRegistry::modifiers().size(), ErrorRegistry::modifiers().size()};
        case InjectableType::TypeAnnotation:
            return {ConstructRegistry::type_annotations().size(), ErrorRegistry::type_annotations().size()};
        case InjectableType::WeakStatement:
            return {ConstructRegistry::returns().size(), ErrorRegistry::returns().size()};
        case InjectableType::StrongStatement:
            {
                size_t c_total = ConstructRegistry::assignments().size() +
                    ConstructRegistry::reassignments().size() +
                    ConstructRegistry::expr_stmts().size();
                size_t e_total = ErrorRegistry::assignments().size() +
                    ErrorRegistry::reassignments().size() +
                    ErrorRegistry::expr_stmts().size();
                return {c_total, e_total};
            }
        case InjectableType::TopLevel:
            return {0, 0};
        }
        return {0, 0};
    }

    class FrameworkDiagnosticsTest : public TestingFrameworkTestBase
    {
    protected:
        std::map<std::pair<InjectableType, std::vector<std::string_view>>, size_t> expansion_cache;

        size_t get_expansion_count(InjectableType type, const std::vector<std::string_view>& skip_contexts)
        {
            auto key = std::make_pair(type, skip_contexts);
            if (expansion_cache.contains(key)) return expansion_cache[key];

            size_t count = ExpansionCalculator::compute_expected_expansions(type, skip_contexts);
            expansion_cache[key] = count;
            return count;
        }

        size_t count_happy_executions(InjectableType type, const UniversalVerifier& dummy_verifier)
        {
            size_t count = 0;
            size_t augmentations = ParserTestBase::get_augmentation_count(type, "dummy", dummy_verifier, "test", {});

            auto add = [&](const auto& registry)
            {
                count += registry.size() * augmentations * get_expansion_count(type, {});
            };

            switch (type)
            {
            case InjectableType::Identifier: break;
            case InjectableType::Import: add(ConstructRegistry::imports());
                break;
            case InjectableType::Directive: add(ConstructRegistry::directives());
                break;
            case InjectableType::Function: add(ConstructRegistry::functions());
                break;
            case InjectableType::Extension: add(ConstructRegistry::extensions());
                break;
            case InjectableType::Struct: add(ConstructRegistry::structs());
                break;
            case InjectableType::Enum: add(ConstructRegistry::enums());
                break;
            case InjectableType::TypeAlias: add(ConstructRegistry::aliases());
                break;
            case InjectableType::Expression: add(ConstructRegistry::expressions());
                break;
            case InjectableType::Modifier: add(ConstructRegistry::modifiers());
                break;
            case InjectableType::TypeAnnotation: add(ConstructRegistry::type_annotations());
                break;
            case InjectableType::WeakStatement: add(ConstructRegistry::returns());
                break;
            case InjectableType::StrongStatement:
                add(ConstructRegistry::assignments());
                add(ConstructRegistry::reassignments());
                add(ConstructRegistry::expr_stmts());
                break;
            case InjectableType::TopLevel: break;
            }
            return count;
        }

        size_t count_sad_executions(InjectableType type, const UniversalVerifier& dummy_verifier)
        {
            size_t count = 0;
            size_t augmentations = ParserTestBase::get_augmentation_count(type, "dummy", dummy_verifier, "test", {});

            auto add = [&](const auto& registry)
            {
                for (const auto& entry : registry)
                {
                    count += augmentations * get_expansion_count(type, entry.skip_contexts);
                }
            };

            switch (type)
            {
            case InjectableType::Identifier: break;
            case InjectableType::Import: add(ErrorRegistry::imports());
                break;
            case InjectableType::Directive: add(ErrorRegistry::directives());
                break;
            case InjectableType::Function: add(ErrorRegistry::functions());
                break;
            case InjectableType::Extension: add(ErrorRegistry::extensions());
                break;
            case InjectableType::Struct: add(ErrorRegistry::structs());
                break;
            case InjectableType::Enum: add(ErrorRegistry::enums());
                break;
            case InjectableType::TypeAlias: add(ErrorRegistry::aliases());
                break;
            case InjectableType::Expression: add(ErrorRegistry::expressions());
                break;
            case InjectableType::Modifier: add(ErrorRegistry::modifiers());
                break;
            case InjectableType::TypeAnnotation: add(ErrorRegistry::type_annotations());
                break;
            case InjectableType::WeakStatement: add(ErrorRegistry::returns());
                break;
            case InjectableType::StrongStatement:
                add(ErrorRegistry::assignments());
                add(ErrorRegistry::reassignments());
                add(ErrorRegistry::expr_stmts());
                break;
            case InjectableType::TopLevel: break;
            }
            return count;
        }
    };

    TEST_F(FrameworkDiagnosticsTest, OutputDiagnostics)
    {
        DumpWriter writer("framework_details.txt", "TestDiagnostics");
        ASSERT_TRUE(writer.is_open()) << "Could not open dump file!";
        auto& out = writer.out();

        out << "====================================================================================\n";
        out << "                     PARSER TESTING FRAMEWORK DIAGNOSTICS\n";
        out << "====================================================================================\n\n";

        out << "=== 1. REGISTRY ITEM COUNTS ===\n\n";
        out << std::left << std::setw(20) << "Construct Type"
            << "| " << std::setw(12) << "Happy Path"
            << "| " << "Sad Path\n";
        out << "------------------------------------------------\n";

        auto print_registry_row = [&](const std::string& name, size_t happy, size_t sad)
        {
            out << std::left << std::setw(20) << name
                << "| " << std::setw(12) << happy
                << "| " << sad << "\n";
        };

        print_registry_row("Imports", ConstructRegistry::imports().size(), ErrorRegistry::imports().size());
        print_registry_row("Directives", ConstructRegistry::directives().size(), ErrorRegistry::directives().size());
        print_registry_row("Functions", ConstructRegistry::functions().size(), ErrorRegistry::functions().size());
        print_registry_row("Extensions", ConstructRegistry::extensions().size(), ErrorRegistry::extensions().size());
        print_registry_row("Structs", ConstructRegistry::structs().size(), ErrorRegistry::structs().size());
        print_registry_row("Enums", ConstructRegistry::enums().size(), ErrorRegistry::enums().size());
        print_registry_row("Aliases", ConstructRegistry::aliases().size(), ErrorRegistry::aliases().size());
        print_registry_row("Assignments", ConstructRegistry::assignments().size(), ErrorRegistry::assignments().size());
        print_registry_row("Reassignments", ConstructRegistry::reassignments().size(),
                           ErrorRegistry::reassignments().size());
        print_registry_row("Returns", ConstructRegistry::returns().size(), ErrorRegistry::returns().size());
        print_registry_row("Expr Stmts", ConstructRegistry::expr_stmts().size(), ErrorRegistry::expr_stmts().size());
        print_registry_row("Expressions", ConstructRegistry::expressions().size(), ErrorRegistry::expressions().size());
        print_registry_row("Modifiers", ConstructRegistry::modifiers().size(), ErrorRegistry::modifiers().size());
        print_registry_row("Type Annotations", ConstructRegistry::type_annotations().size(),
                           ErrorRegistry::type_annotations().size());
        out << "\n";

        const std::vector<std::pair<InjectableType, UniversalVerifier>> injectables_with_verifiers = {
            {InjectableType::Import, ImportVerifier{}},
            {InjectableType::Directive, DirectiveVerifier{}},
            {InjectableType::Function, FuncVerifier{}},
            {InjectableType::Extension, ExtVerifier{}},
            {InjectableType::Struct, StructVerifier{}},
            {InjectableType::Enum, EnumVerifier{}},
            {InjectableType::TypeAlias, AliasVerifier{}},
            {InjectableType::Expression, ExprVerifier{}},
            {InjectableType::Modifier, ModifierVerifier{}},
            {InjectableType::TypeAnnotation, TypeVerifier{}},
            {InjectableType::WeakStatement, ReturnVerifier{}},
            {InjectableType::StrongStatement, StmtVerifier{}},
            {InjectableType::TopLevel, NullVerifier{}}
        };

        out << "=== 2. CONTEXTS ===\n\n";
        out << std::left << std::setw(18) << "Injectable Type"
            << "| " << std::setw(16) << "Total Contexts"
            << "| " << "Output Breakdown\n";
        out << "------------------------------------------------------------------------------------\n";

        for (const auto& [type, dummy_verifier] : injectables_with_verifiers)
        {
            auto contexts = ContextRegistry::get_all_for(type);

            std::map<InjectableType, int> output_breakdown;
            for (const auto& ctx : contexts)
            {
                output_breakdown[ctx.output_type]++;
            }

            std::string breakdown_str;
            for (auto it = output_breakdown.begin(); it != output_breakdown.end(); ++it)
            {
                breakdown_str += std::string(to_string(it->first)) + ": " + std::to_string(it->second);
                if (std::next(it) != output_breakdown.end())
                {
                    breakdown_str += ", ";
                }
            }

            if (breakdown_str.empty()) breakdown_str = "None";

            out << std::left << std::setw(18) << to_string(type)
                << "| " << std::setw(16) << contexts.size()
                << "| " << breakdown_str << "\n";
        }
        out << "\n";

        out << "=== 3. AUGMENTATIONS ===\n\n";
        out << std::left << std::setw(18) << "Injectable Type"
            << "| " << "Variations per Item\n";
        out << "----------------------------------------\n";

        for (const auto& [type, dummy_verifier] : injectables_with_verifiers)
        {
            size_t variations = ParserTestBase::get_augmentation_count(type, "dummy_snippet", dummy_verifier, "test_group");
            out << std::left << std::setw(18) << to_string(type)
                << "| " << variations << "\n";
        }
        out << "\n";

        out << "=== 4. TOTAL SENTINEL POOL SIZES ===\n\n";

        size_t block_pool_size =
            ConstructRegistry::assignments().size() +
            ConstructRegistry::reassignments().size() +
            ConstructRegistry::expr_stmts().size() +
            ConstructRegistry::returns().size();

        size_t top_level_pool_size =
            ConstructRegistry::assignments().size() +
            ConstructRegistry::reassignments().size() +
            ConstructRegistry::expr_stmts().size() +
            ConstructRegistry::imports().size() +
            ConstructRegistry::functions().size() +
            ConstructRegistry::extensions().size() +
            ConstructRegistry::structs().size() +
            ConstructRegistry::enums().size() +
            ConstructRegistry::aliases().size() +
            ConstructRegistry::directives().size();

        out << std::left << std::setw(22) << "Pool Type"
            << "| " << "Available Snippets\n";
        out << "----------------------------------------\n";
        out << std::left << std::setw(22) << "Block Sentinels" << "| " << block_pool_size << "\n";
        out << std::left << std::setw(22) << "Top Level Sentinels" << "| " << top_level_pool_size << "\n\n";

        out << "=== 5. TOTAL TEST EXECUTION ESTIMATE ===\n\n";

        auto policy = ExpansionPolicy::current();
        out << " -> ENV Configuration (Max Depth): " << policy.max_depth << "\n";
        out << " -> ENV Configuration (Max Recursion): " << policy.max_recursion << "\n\n";

        size_t grand_total_tests = 0;

        out << std::left << std::setw(16) << "Type"
            << "| " << std::setw(6) << "Happy"
            << "| " << std::setw(6) << "Sad"
            << "| " << std::setw(15) << "Augmentations"
            << "| " << std::setw(11) << "Expansions"
            << "| " << "Total Executions\n";
        out << "--------------------------------------------------------------------------------\n";

        for (const auto& [type, dummy_verifier] : injectables_with_verifiers)
        {
            auto [happy_cnt, sad_cnt] = get_registry_counts(type);
            size_t aug_cnt = ParserTestBase::get_augmentation_count(type, "dummy", dummy_verifier, "test");
            size_t expansions = ExpansionCalculator::compute_expected_expansions(type);

            size_t happy_executions = count_happy_executions(type, dummy_verifier);
            size_t sad_executions = count_sad_executions(type, dummy_verifier);
            size_t run_total = happy_executions + sad_executions;

            grand_total_tests += run_total;

            out << std::left << std::setw(16) << to_string(type)
                << "| " << std::setw(6) << happy_cnt
                << "| " << std::setw(6) << sad_cnt
                << "| " << std::setw(15) << aug_cnt
                << "| " << std::setw(11) << expansions
                << "| " << run_total << "\n";
        }

        out << "--------------------------------------------------------------------------------\n";
        out << "GRAND TOTAL PARSER TESTS GENERATED: " << grand_total_tests << "\n";

        out << "\n====================================================================================\n";
    }
}
