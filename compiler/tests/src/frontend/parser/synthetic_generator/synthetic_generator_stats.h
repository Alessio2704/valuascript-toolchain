#pragma once
#include <string>

namespace valuascript::compiler::test
{
    struct SyntheticGenerationStats
    {
        int max_ast_depth = 0;
        int total_nodes_generated = 0;
        int registry_fallbacks = 0;
        int synthesized_from_scratch = 0;

        struct
        {
            int expressions = 0;
            int type_annotations = 0;
            int statements = 0;
            int return_stmts = 0;
            int modifiers = 0;
            int functions = 0;
            int structs = 0;
            int enums = 0;
            int type_aliases = 0;
            int extensions = 0;
            int imports = 0;
            int directives = 0;
        } pieces_rolled;

        struct
        {
            int total_generated = 0;
            int with_arguments = 0;
            int attached_to_functions = 0;
            int attached_to_structs = 0;
            int attached_to_enums = 0;
            int attached_to_enum_cases = 0;
            int attached_to_struct_fields = 0;
            int attached_to_parameters = 0;
            int attached_to_assignments_outside = 0;
            int attached_to_assignments_inside = 0;
            int attached_to_assignments_both = 0;
        } modifiers;

        struct
        {
            int total = 0;
            int with_docstrings = 0;
            int total_parameters = 0;
            int parameters_with_defaults = 0;
            int total_body_statements = 0;
        } functions;

        struct
        {
            int total_structs = 0;
            int total_fields = 0;
            int total_enums = 0;
            int total_cases = 0;
            int cases_with_values = 0;
        } data_structures;

        struct
        {
            int single_assignments = 0;
            int multi_assignments = 0;
            int explicit_type_annotations = 0;

            int reassignments_id = 0;
            int reassignments_dot = 0;
            int reassignments_bracket = 0;
            int reassignments_self = 0;

            int expression_statements = 0;

            int total_returns = 0;
            int multi_returns = 0;
            int total_return_values = 0;
        } statements;

        struct
        {
            int binary = 0;
            int unary = 0;
            int dot = 0;
            int bracket = 0;
            int call = 0;
            int grouping = 0;
            int dict_expr = 0;
            int tuple_expr = 0;
            int tensor_expr = 0;
            int conditional = 0;
            int leaf_fallback = 0;

            int switch_expr = 0;
            int switch_defaults = 0;
            int total_switch_cases = 0;
            int cases_with_multiple_labels = 0;
        } expressions;

        struct
        {
            int fallback_to_any = 0;
            int tuple_types = 0;
            int list_types = 0;
        } types;

        [[nodiscard]] std::string dump_report(size_t seed) const;
    };
}
