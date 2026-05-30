#pragma once
#include <utility>
#include <string>
#include <sstream>

namespace valuascript::compiler::test
{
    enum class StatementType { SingleAssign, MultiAssign, Reassign, ExprStmt };

    enum class ReassignTargetFlavor { Id, Dot, Bracket, SelfDot };

    enum class ExpressionType { Binary, Unary, Dot, Bracket, Call, Grouping, Switch, Dict, Tuple, Tensor, Conditional };

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
            double modifiers = 0.0;
            double types = 0.0;
            double expressions = 0.0;
            double returns = 0.0;
            double statements = 0.0;
            double functions = 0.0;
            double structs = 0.0;
            double enums = 0.0;
            double type_aliases = 0.0;
            double imports = 0.0;
            double directives = 0.0;
        } registry;

        struct FeatureProbabilities
        {
            double enum_case_has_value = 0.0;
            double assignment_has_explicit_type = 0.0;
            double assignment_has_let_modifiers = 0.0;
            double assignment_target_has_modifiers = 0.0;
            double func_has_docstring = 0.0;
            double directive_has_value = 0.0;
            double type_fallback_to_any = 0.0;
            double type_is_tuple_vs_generic = 0.0;
            double switch_case_has_multiple_labels = 0.0;
            double return_has_multiple_values = 0.0;
        } features;

        struct SizeRanges
        {
            std::pair<int, int> modifiers_count = {0, 0};
            std::pair<int, int> standalone_modifiers_count = {0, 0};
            std::pair<int, int> modifier_arguments = {0, 0};
            std::pair<int, int> function_statements = {0, 0};
            std::pair<int, int> function_parameters = {0, 0};
            std::pair<int, int> struct_fields = {0, 0};
            std::pair<int, int> enum_cases = {0, 0};
            std::pair<int, int> multi_assign_targets = {0, 0};
            std::pair<int, int> switch_cases = {0, 0};
            std::pair<int, int> switch_case_labels = {0, 0};
            std::pair<int, int> dict_elements = {0, 0};
            std::pair<int, int> tuple_elements = {0, 0};
            std::pair<int, int> tensor_elements = {0, 0};
            std::pair<int, int> return_values = {0, 0};

            int max_type_depth = 0;
            int max_ast_depth = 0;
            std::pair<int, int> expansion_policy_max_steps_retries = {0, 0};
        } sizes;

        struct Weights
        {
            struct
            {
                double expression = 0.0;
                double type_annotation = 0.0;
                double statement = 0.0;
                double return_stmt = 0.0;
                double modifier = 0.0;
                double function_def = 0.0;
                double struct_def = 0.0;
                double enum_def = 0.0;
                double type_alias = 0.0;
                double import_stmt = 0.0;
                double directive = 0.0;
            } top_level_constructs;

            struct
            {
                double single_assign = 0.0;
                double multi_assign = 0.0;
                double reassign = 0.0;
                double expr_stmt = 0.0;
            } statement_types;

            struct
            {
                double id = 0.0;
                double dot = 0.0;
                double bracket = 0.0;
                double self_dot = 0.0;
            } reassign_target_flavors;

            struct
            {
                double binary = 0.0;
                double unary = 0.0;
                double dot = 0.0;
                double bracket = 0.0;
                double call = 0.0;
                double grouping = 0.0;
                double switch_expr = 0.0;
                double dict_expr = 0.0;
                double tuple_expr = 0.0;
                double tensor_expr = 0.0;
                double conditional = 0.0;
            } expression_types;

            struct
            {
                double assignment = 0.0;
                double reassignment = 0.0;
                double expr_stmt = 0.0;
            } harvest_statement_types;
        } weights;

        static SyntheticGeneratorConfig default_fuzzing_config()
        {
            SyntheticGeneratorConfig c;
            c.registry.modifiers = 0.5;
            c.registry.types = 0.5;
            c.registry.expressions = 0.5;
            c.registry.returns = 0.5;
            c.registry.statements = 0.5;
            c.registry.functions = 0.5;
            c.registry.structs = 0.5;
            c.registry.enums = 0.5;
            c.registry.type_aliases = 0.5;
            c.registry.imports = 0.5;
            c.registry.directives = 0.5;

            c.features.enum_case_has_value = 0.5;
            c.features.assignment_has_explicit_type = 0.3;
            c.features.assignment_has_let_modifiers = 0.5;
            c.features.assignment_target_has_modifiers = 0.5;
            c.features.func_has_docstring = 0.3;
            c.features.directive_has_value = 0.5;
            c.features.type_fallback_to_any = 0.333333;
            c.features.type_is_tuple_vs_generic = 0.5;
            c.features.switch_case_has_multiple_labels = 0.3;
            c.features.return_has_multiple_values = 0.2;

            c.sizes.modifiers_count = {0, 2};
            c.sizes.standalone_modifiers_count = {1, 3};
            c.sizes.modifier_arguments = {0, 3};
            c.sizes.function_statements = {1, 4};
            c.sizes.function_parameters = {0, 3};
            c.sizes.struct_fields = {1, 3};
            c.sizes.enum_cases = {1, 3};
            c.sizes.multi_assign_targets = {2, 5};
            c.sizes.switch_cases = {1, 3};
            c.sizes.switch_case_labels = {2, 4};
            c.sizes.dict_elements = {1, 3};
            c.sizes.tuple_elements = {2, 4};
            c.sizes.tensor_elements = {1, 3};
            c.sizes.return_values = {1, 3};
            c.sizes.max_type_depth = 2;
            c.sizes.max_ast_depth = EXPANSION_DEPTH;
            c.sizes.expansion_policy_max_steps_retries = {20, 5};

            c.weights.top_level_constructs.expression = 1.0;
            c.weights.top_level_constructs.type_annotation = 1.0;
            c.weights.top_level_constructs.statement = 1.0;
            c.weights.top_level_constructs.return_stmt = 1.0;
            c.weights.top_level_constructs.modifier = 1.0;
            c.weights.top_level_constructs.function_def = 1.0;
            c.weights.top_level_constructs.struct_def = 1.0;
            c.weights.top_level_constructs.enum_def = 1.0;
            c.weights.top_level_constructs.type_alias = 1.0;
            c.weights.top_level_constructs.import_stmt = 1.0;
            c.weights.top_level_constructs.directive = 1.0;

            c.weights.statement_types.single_assign = 1.0;
            c.weights.statement_types.multi_assign = 1.0;
            c.weights.statement_types.reassign = 1.0;
            c.weights.statement_types.expr_stmt = 1.0;

            c.weights.reassign_target_flavors.id = 1.0;
            c.weights.reassign_target_flavors.dot = 1.0;
            c.weights.reassign_target_flavors.bracket = 1.0;
            c.weights.reassign_target_flavors.self_dot = 1.0;

            c.weights.expression_types.binary = 1.0;
            c.weights.expression_types.unary = 1.0;
            c.weights.expression_types.dot = 1.0;
            c.weights.expression_types.bracket = 1.0;
            c.weights.expression_types.call = 1.0;
            c.weights.expression_types.grouping = 1.0;
            c.weights.expression_types.switch_expr = 1.0;
            c.weights.expression_types.dict_expr = 1.0;
            c.weights.expression_types.tuple_expr = 1.0;
            c.weights.expression_types.tensor_expr = 1.0;
            c.weights.expression_types.conditional = 1.0;

            c.weights.harvest_statement_types.assignment = 1.0;
            c.weights.harvest_statement_types.reassignment = 1.0;
            c.weights.harvest_statement_types.expr_stmt = 1.0;

            return c;
        }

        [[nodiscard]] std::string generate_report(const size_t seed) const
        {
            std::stringstream out;
            out << "/* " << std::string(77, '=') << "\n";
            out << " * VALUASCRIPT SYNTHETIC GENERATOR - EXPERIMENT REPORT\n";
            out << " * Seed: " << seed << "\n";
            out << " * " << std::string(75, '-') << "\n";

            out << " * [REGISTRY PROBABILITIES]\n";
            out << " *   Mods: " << registry.modifiers << " | Types: " << registry.types << " | Exprs: " << registry.
                expressions << "\n";
            out << " *   Stmts: " << registry.statements << " | Funcs: " << registry.functions << " | Structs: " <<
                registry.structs << "\n";
            out << " *   Enums: " << registry.enums << " | Aliases: " << registry.type_aliases << " | Returns: " <<
                registry.returns
                << " | Imports: " << registry.imports << " | Dirs: " << registry.directives << "\n\n";

            out << " * [FEATURE PROBABILITIES]\n";
            out << " *   Enum Vals: " << features.enum_case_has_value << " | Explicit Let Type: " << features.
                assignment_has_explicit_type << "\n";
            out << " *   Let Modifiers: " << features.assignment_has_let_modifiers << " | Target Modifiers: " <<
                features.assignment_target_has_modifiers << " | Docstrings: " << features.func_has_docstring << "\n";
            out << " *   Directive Vals: " << features.directive_has_value << " | Fallback (any): " << features.
                type_fallback_to_any << "\n";
            out << " *   Tuple vs Generic Type: " << features.type_is_tuple_vs_generic << "\n";
            out << " *   Multi-Label Switch Cases: " << features.switch_case_has_multiple_labels <<
                " | Multi-Value Returns: " << features.return_has_multiple_values << "\n\n";

            out << " * [SIZE RANGES]\n";
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
                second << "]\n";
            out << " *   Switch Cases: [" << sizes.switch_cases.first << "," << sizes.switch_cases.second <<
                "] | Switch Case Labels: [" << sizes.switch_case_labels.first << "," << sizes.switch_case_labels.second
                << "]\n";
            out << " *   Dict Elements: [" << sizes.dict_elements.first << "," << sizes.dict_elements.second << "]\n";
            out << " *   Tuple Elements: [" << sizes.tuple_elements.first << "," << sizes.tuple_elements.second <<
                "] | Tensor Elements: [" << sizes.tensor_elements.first << "," << sizes.tensor_elements.second << "]\n";
            out << " *   Return Values: [" << sizes.return_values.first << "," << sizes.return_values.second << "]\n";
            out << " *   Max Type Depth: " << sizes.max_type_depth << " | Max AST Depth: " << sizes.max_ast_depth <<
                " | Expansion Policy: {" << sizes.expansion_policy_max_steps_retries.first << ", " << sizes.
                expansion_policy_max_steps_retries.second << "}\n\n";

            out << " * [TOP-LEVEL WEIGHTS]\n";
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
            out << " *   Expr Types: [Bin: " << weights.expression_types.binary << ", Un: " << weights.expression_types.
                unary << ", Dot: " << weights.expression_types.dot << ", Brk: " << weights.expression_types.bracket <<
                ", Call: " << weights.expression_types.call << ", Grp: " << weights.expression_types.grouping <<
                ", Sw: " << weights.expression_types.switch_expr << ", Dict: " << weights.expression_types.dict_expr <<
                ", Tup: " << weights.expression_types.tuple_expr << ", Ten: " << weights.expression_types.tensor_expr <<
                ", Cond: " << weights.expression_types.conditional << "]\n";
            out << " *   Harvest Ratio: [Let: " << weights.harvest_statement_types.assignment << ", Re: " << weights.
                harvest_statement_types.reassignment << ", Call: " << weights.harvest_statement_types.expr_stmt <<
                "]\n";
            out << " " << std::string(77, '=') << " */\n\n";

            return out.str();
        }
    };
}
