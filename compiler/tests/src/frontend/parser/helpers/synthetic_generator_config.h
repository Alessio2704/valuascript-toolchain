#pragma once
#include <utility>
#include <string>
#include <sstream>

namespace valuascript::compiler::test
{
    enum class StatementType { SingleAssign, MultiAssign, Reassign, ExprStmt };

    enum class ReassignTargetFlavor { Id, Dot, Bracket, SelfDot };

    enum class ExpressionType { Binary, Dot, Bracket, Call, Grouping };

    enum class HarvestStatementType { Assignment, Reassignment, ExprStmt };

    enum class TopLevelConstruct
    {
        None,
        Expression, TypeAnnotation, Statement, ReturnStmt, Modifier,
        FunctionDef, StructDef, EnumDef, TypeAlias, ImportStmt, Directive,
        Count
    };

    struct SyntheticGeneratorConfig
    {
        struct RegistryProbabilities
        {
            double modifiers = 0.5;
            double types = 0.5;
            double expressions = 0.5;
            double statements = 0.5;
            double functions = 0.5;
            double structs = 0.5;
            double enums = 0.5;
            double type_aliases = 0.5;
            double imports = 0.5;
            double directives = 0.5;
        } registry;

        struct FeatureProbabilities
        {
            double enum_case_has_value = 0.5;
            double assignment_has_explicit_type = 0.3;
            double func_has_docstring = 0.3;
            double directive_has_value = 0.5;
            double type_fallback_to_any = 0.333333;
            double type_is_tuple_vs_generic = 0.5;
        } features;

        struct SizeRanges
        {
            std::pair<int, int> modifiers_count = {0, 2};
            std::pair<int, int> standalone_modifiers_count = {1, 3};
            std::pair<int, int> modifier_arguments = {0, 3};
            std::pair<int, int> function_statements = {1, 4};
            std::pair<int, int> function_parameters = {0, 3};
            std::pair<int, int> struct_fields = {1, 3};
            std::pair<int, int> enum_cases = {1, 3};
            std::pair<int, int> multi_assign_targets = {2, 5};
        } sizes;

        struct Weights
        {
            struct
            {
                double expression = 1.0;
                double type_annotation = 1.0;
                double statement = 1.0;
                double return_stmt = 1.0;
                double modifier = 1.0;
                double function_def = 1.0;
                double struct_def = 1.0;
                double enum_def = 1.0;
                double type_alias = 1.0;
                double import_stmt = 1.0;
                double directive = 1.0;
            } top_level_constructs;

            struct
            {
                double single_assign = 1.0;
                double multi_assign = 1.0;
                double reassign = 1.0;
                double expr_stmt = 1.0;
            } statement_types;

            struct
            {
                double id = 1.0;
                double dot = 1.0;
                double bracket = 1.0;
                double self_dot = 1.0;
            } reassign_target_flavors;

            struct
            {
                double binary = 1.0;
                double dot = 1.0;
                double bracket = 1.0;
                double call = 1.0;
                double grouping = 1.0;
            } expression_types;

            struct
            {
                double assignment = 1.0;
                double reassignment = 1.0;
                double expr_stmt = 1.0;
            } harvest_statement_types;
        } weights;

        [[nodiscard]] std::string generate_report(const size_t seed) const
        {
            std::stringstream out;
            out << "/* " << std::string(77, '=') << "\n";
            out << " * VALUASCRIPT SYNTHETIC GENERATOR - EXPERIMENT REPORT\n";
            out << " * Seed: " << seed << "\n";
            out << " * " << std::string(75, '-') << "\n";

            out << " * [REGISTRY PROBABILITIES (Chance to pool from existing tests)]\n";
            out << " *   Mods: " << registry.modifiers << " | Types: " << registry.types << " | Exprs: " << registry.
                expressions << "\n";
            out << " *   Stmts: " << registry.statements << " | Funcs: " << registry.functions << " | Structs: " <<
                registry.structs << "\n";
            out << " *   Enums: " << registry.enums << " | Aliases: " << registry.type_aliases << " | Imports: " <<
                registry.imports << " | Dirs: " << registry.directives << "\n\n";

            out << " * [FEATURE PROBABILITIES (Coin flips)]\n";
            out << " *   Enum Vals: " << features.enum_case_has_value << " | Explicit Let Type: " << features.
                assignment_has_explicit_type << "\n";
            out << " *   Docstrings: " << features.func_has_docstring << " | Directive Vals: " << features.
                directive_has_value << "\n";
            out << " *   Fallback (any): " << features.type_fallback_to_any << " | Tuple vs Generic Type: " << features.
                type_is_tuple_vs_generic << "\n\n";

            out << " * [SIZE RANGES {min, max}]\n";
            out << " *   Mods: [" << sizes.modifiers_count.first << "," << sizes.modifiers_count.second <<
                "] | Mod Args: [" << sizes.modifier_arguments.first << "," << sizes.modifier_arguments.second << "]\n";
            out << " *   Standalone Mods: [" << sizes.standalone_modifiers_count.first << "," << sizes.
                standalone_modifiers_count.second << "]\n";
            out << " *   Func Params: [" << sizes.function_parameters.first << "," << sizes.function_parameters.second
                << "] | Func Stmts: [" << sizes.function_statements.first << "," << sizes.function_statements.second <<
                "]\n";
            out << " *   Struct Fields: [" << sizes.struct_fields.first << "," << sizes.struct_fields.second <<
                "] | Enum Cases: [" << sizes.enum_cases.first << "," << sizes.enum_cases.second << "]\n";
            out << " *   Multi-Let Targets: [" << sizes.multi_assign_targets.first << "," << sizes.multi_assign_targets.
                second << "]\n\n";

            out << " * [TOP-LEVEL WEIGHTS (Stochastic distribution)]\n";
            out << " *   Expr: " << weights.top_level_constructs.expression << " | Type: " << weights.
                top_level_constructs.type_annotation << " | Stmt: " << weights.top_level_constructs.statement << "\n";
            out << " *   Return: " << weights.top_level_constructs.return_stmt << " | Mod: " << weights.
                top_level_constructs.modifier << " | Func: " << weights.top_level_constructs.function_def << "\n";
            out << " *   Struct: " << weights.top_level_constructs.struct_def << " | Enum: " << weights.
                top_level_constructs.enum_def << " | Alias: " << weights.top_level_constructs.type_alias << "\n";
            out << " *   Import: " << weights.top_level_constructs.import_stmt << " | Dir: " << weights.
                top_level_constructs.directive << "\n\n";

            out << " * [INNER WEIGHTS]\n";
            out << " *   Stmt Types: [Let: " << weights.statement_types.single_assign << ", Multi: " << weights.
                statement_types.multi_assign << ", Re: " << weights.statement_types.reassign << ", Call: " << weights.
                statement_types.expr_stmt << "]\n";
            out << " *   Reassign Targets: [ID: " << weights.reassign_target_flavors.id << ", Dot: " << weights.
                reassign_target_flavors.dot << ", Brk: " << weights.reassign_target_flavors.bracket << ", Self: " <<
                weights.reassign_target_flavors.self_dot << "]\n";
            out << " *   Expr Types: [Bin: " << weights.expression_types.binary << ", Dot: " << weights.expression_types
                .dot << ", Brk: " << weights.expression_types.bracket << ", Call: " << weights.expression_types.call <<
                ", Grp: " << weights.expression_types.grouping << "]\n";
            out << " *   Harvest Ratio: [Let: " << weights.harvest_statement_types.assignment << ", Re: " << weights.
                harvest_statement_types.reassignment << ", Call: " << weights.harvest_statement_types.expr_stmt <<
                "]\n";
            out << " " << std::string(77, '=') << " */\n\n";

            return out.str();
        }
    };
}
