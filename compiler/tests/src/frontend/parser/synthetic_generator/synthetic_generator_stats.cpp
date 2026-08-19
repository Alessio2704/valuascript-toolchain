#include "synthetic_generator_stats.h"
#include <sstream>
#include <iomanip>

namespace valuascript::compiler::test
{
    std::string SyntheticGenerationStats::dump_report(size_t seed) const
    {
        std::stringstream out;

        auto sep = [&]() { out << "// +" << std::string(70, '-') << "+\n"; };

        auto title = [&](const std::string& text)
        {
            out << "// | " << std::left << std::setw(68) << text << " |\n";
        };

        auto row1 = [&](const std::string& label, auto val)
        {
            out << "// | " << std::left << std::setw(50) << label
                << " : " << std::right << std::setw(15) << val << " |\n";
        };

        auto row1_f = [&](const std::string& label, float val)
        {
            std::stringstream val_ss;
            val_ss << std::fixed << std::setprecision(1) << val;
            out << "// | " << std::left << std::setw(50) << label
                << " : " << std::right << std::setw(15) << val_ss.str() << " |\n";
        };

        auto row2 = [&](const std::string& l1, int v1, const std::string& l2, int v2)
        {
            out << "// | " << std::left << std::setw(24) << l1
                << " : " << std::right << std::setw(6) << v1
                << " | " << std::left << std::setw(24) << l2
                << " : " << std::right << std::setw(5) << v2 << " |\n";
        };

        sep();
        title("SYNTHETIC GENERATION STATS (Seed: " + std::to_string(seed) + ")");

        sep();
        title("[ META ]");
        row1("Max AST Depth", max_ast_depth);
        row1("Total Nodes Generated", total_nodes_generated);
        row1("Synthesized from scratch", synthesized_from_scratch);
        row1("Pulled from registry", registry_fallbacks);

        sep();
        title("[ PIECES ROLLED (Pre-Wrapping) ]");
        row2("Functions", pieces_rolled.functions, "Structs", pieces_rolled.structs);
        row2("Enums", pieces_rolled.enums, "TypeAliases", pieces_rolled.type_aliases);
        row2("Naked Statements", pieces_rolled.statements, "Naked Expressions", pieces_rolled.expressions);
        row2("Return Stmts", pieces_rolled.return_stmts, "Standalone Mods", pieces_rolled.modifiers);
        row2("Extensions", pieces_rolled.extensions, "Imports", pieces_rolled.imports);
        row1("Directives", pieces_rolled.directives);

        sep();
        title("[ MODIFIER USAGE ]");
        row2("Total Generated", modifiers.total_generated, "With Arguments", modifiers.with_arguments);
        row2("Attached to Functions", modifiers.attached_to_functions, "Attached to Structs",
             modifiers.attached_to_structs);
        row2("Attached to Enums", modifiers.attached_to_enums, "Attached to Enum Cases",
             modifiers.attached_to_enum_cases);
        row2("Attached to Fields", modifiers.attached_to_struct_fields, "Attached to Params",
             modifiers.attached_to_parameters);
        title("--- Assignment Attachments ---");
        row2("Outside Only", modifiers.attached_to_assignments_outside, "Inside Only",
             modifiers.attached_to_assignments_inside);
        row1("Both (Outside & Inside)", modifiers.attached_to_assignments_both);

        sep();
        title("[ FUNCTIONS ]");
        row2("Total Generated", functions.total, "With Docstrings", functions.with_docstrings);
        row2("Total Parameters", functions.total_parameters, "Params w/ Defaults", functions.parameters_with_defaults);
        row1_f("Average Params per Func", functions.total ? static_cast<float>(functions.total_parameters) / static_cast<float>(functions.total) : 0.0f);
        row1("Total Body Statements", functions.total_body_statements);
        row1_f("Average Stmts per Func",
               functions.total ? static_cast<float>(functions.total_body_statements) / static_cast<float>(functions.total) : 0.0f);

        sep();
        title("[ DATA STRUCTURES ]");
        row2("Total Structs", data_structures.total_structs, "Struct Fields Total", data_structures.total_fields);
        row2("Total Enums", data_structures.total_enums, "Enum Cases Total", data_structures.total_cases);
        row1("Cases with Explicit Vals", data_structures.cases_with_values);

        sep();
        title("[ STATEMENTS ]");
        row2("Single Assignments", statements.single_assignments, "Multi Assignments", statements.multi_assignments);
        row1("Explicit Type Annotations", statements.explicit_type_annotations);
        row1("Total Reassignments",
             statements.reassignments_id + statements.reassignments_dot + statements.reassignments_bracket + statements.
             reassignments_self);
        row2("  -> ID", statements.reassignments_id, "  -> Dot", statements.reassignments_dot);
        row2("  -> Bracket", statements.reassignments_bracket, "  -> Self", statements.reassignments_self);
        row1("Expression Statements", statements.expression_statements);
        title("--- Returns ---");
        row2("Total Returns", statements.total_returns, "Multi-Returns", statements.multi_returns);
        row1("Total Return Values", statements.total_return_values);

        sep();
        title("[ EXPRESSIONS HISTOGRAM ]");
        row2("Binary", expressions.binary, "Unary", expressions.unary);
        row2("Dot", expressions.dot, "Bracket", expressions.bracket);
        row2("Call", expressions.call, "Dict", expressions.dict_expr);
        row2("Tuple", expressions.tuple_expr, "Tensor", expressions.tensor_expr);
        row2("Conditional", expressions.conditional, "Grouping", expressions.grouping);
        row1("Leaf Fallback (Depth)", expressions.leaf_fallback);
        title("--- Switches ---");
        row2("Total Switch Exprs", expressions.switch_expr, "Total Default Cases", expressions.switch_defaults);
        row2("Total Cases", expressions.total_switch_cases, "Multi-Label Cases",
             expressions.cases_with_multiple_labels);

        sep();
        title("[ TYPES ]");
        row2("Fallback to 'any'", types.fallback_to_any, "Tuple Types", types.tuple_types);
        row1("List Types", types.list_types);

        sep();
        out << "\n";
        return out.str();
    }
}
