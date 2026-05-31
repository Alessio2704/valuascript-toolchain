#pragma once

namespace valuascript::compiler::test::ContextNames
{
    inline constexpr const char* FunctionBodyWrapper = "function_body_wrapper";

    inline constexpr const char* ExprSingleAssignment = "single_assignment";
    inline constexpr const char* ExprMultiAssignment = "multi_assignment";
    inline constexpr const char* ExprReassignment = "reassignment";
    inline constexpr const char* ExprReturnStmt = "return_stmt";
    inline constexpr const char* ExprFuncDefDefault = "func_def_default";
    inline constexpr const char* ExprDirectiveNoEq = "directive_no_eq";
    inline constexpr const char* ExprDirectiveEq = "directive_eq";
    inline constexpr const char* ExprEnumCase = "enum_case";
    inline constexpr const char* ExprModifierArg = "modifier_arg";
    inline constexpr const char* ExprTupleElement = "tuple_element";
    inline constexpr const char* ExprTensorElement = "tensor_element";
    inline constexpr const char* ExprDictValue = "dict_value";
    inline constexpr const char* ExprBracketAccessIndex = "bracket_access_index";
    inline constexpr const char* ExprFunctionCallArg = "function_call_arg";
    inline constexpr const char* ExprBinaryLhs = "binary_lhs";
    inline constexpr const char* ExprBinaryRhs = "binary_rhs";
    inline constexpr const char* ExprGrouping = "grouping";
    inline constexpr const char* ExprUnaryGrouping = "unary_grouping";
    inline constexpr const char* ExprAsCallTarget = "as_call_target";
    inline constexpr const char* ExprAsDotTarget = "as_dot_target";
    inline constexpr const char* ExprAsBracketTarget = "as_bracket_target";
    inline constexpr const char* ExprAsSliceTarget = "as_slice_target";
    inline constexpr const char* ExprSwitchCond = "switch_cond";
    inline constexpr const char* ExprSwitchCase = "switch_case";
    inline constexpr const char* ExprIfCond = "if_cond";
    inline constexpr const char* ExprIfThen = "if_then";
    inline constexpr const char* ExprIfElse = "if_else";

    inline constexpr const char* ModBeforeLetSingle = "modifier_before_let_single";
    inline constexpr const char* ModBeforeLetMultiple = "modifier_before_let_multiple";
    inline constexpr const char* ModBeforeLetMultipleWithInner = "modifier_before_let_multiple_with_inner";
    inline constexpr const char* ModBeforeLetMultipleWithBothInner = "modifier_before_let_multiple_with_both_inner";
    inline constexpr const char* ModAssignment = "assignment";
    inline constexpr const char* ModMultiAssignment = "multi_assignment";
    inline constexpr const char* ModImportStatement = "import_statement";
    inline constexpr const char* ModReturnStatement = "return_statement";
    inline constexpr const char* ModSwitchCase = "switch_case";
    inline constexpr const char* ModSwitchDefault = "switch_default";
    inline constexpr const char* ModFunctionDefinition = "function_definition";
    inline constexpr const char* ModFunctionParameter = "function_parameter";
    inline constexpr const char* ModFunctionParameterWithDefault = "function_parameter_with_default";
    inline constexpr const char* ModStructDefinition = "struct_definition";
    inline constexpr const char* ModStructField = "struct_field";
    inline constexpr const char* ModEnumDefinition = "enum_definition";
    inline constexpr const char* ModEnumCase = "enum_case";
    inline constexpr const char* ModTypealiasDefinition = "typealias_definition";
    inline constexpr const char* ModDictItem = "dict_item";

    inline constexpr const char* TypeAssignmentTarget = "assignment_target";
    inline constexpr const char* TypeMultiAssignmentTarget1 = "multi_assignment_target_1";
    inline constexpr const char* TypeMultiAssignmentTarget2 = "multi_assignment_target_2";
    inline constexpr const char* TypeTypealiasTarget = "typealias_target";
    inline constexpr const char* TypeFunctionParameter = "function_parameter";
    inline constexpr const char* TypeFunctionReturn = "function_return";
    inline constexpr const char* TypeFunctionMultiReturn = "function_multi_return";
    inline constexpr const char* TypeStructField = "struct_field";
    inline constexpr const char* TypeEnumUnderlyingType = "enum_underlying_type";
    inline constexpr const char* TypeTupleTypeStart = "tuple_type_start";
    inline constexpr const char* TypeTupleTypeMiddle = "tuple_type_middle";
    inline constexpr const char* TypeTupleTypeEnd = "tuple_type_end";
    inline constexpr const char* TypeGenericTypeStart = "generic_type_start";
    inline constexpr const char* TypeGenericTypeMiddle = "generic_type_middle";
    inline constexpr const char* TypeGenericTypeEnd = "generic_type_end";
}
