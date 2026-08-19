#pragma once

namespace valuascript::compiler::test::ContextNames
{
#define VALUASCRIPT_WRAPPER_CONTEXTS(X) \
    X(FunctionBodyWrapper, "function_body_wrapper") \
    X(ExtensionBodyWrapper, "extension_body_wrapper") \
    X(TopLevelWrapper, "top_level")

#define VALUASCRIPT_EXPRESSION_CONTEXTS(X) \
    X(ExprSingleAssignment, "single_assignment") \
    X(ExprMultiAssignment, "multi_assignment") \
    X(ExprReassignment, "reassignment") \
    X(ExprReturnStmt, "return_stmt") \
    X(ExprFuncDefDefault, "func_def_default") \
    X(ExprDirectiveNoEq, "directive_no_eq") \
    X(ExprDirectiveEq, "directive_eq") \
    X(ExprEnumCase, "enum_case") \
    X(ExprModifierArg, "modifier_arg") \
    X(ExprTupleStart, "tuple_start") \
    X(ExprTupleMiddle, "tuple_middle") \
    X(ExprTupleEnd, "tuple_end") \
    X(ExprTensorStart, "tensor_start") \
    X(ExprTensorMiddle, "tensor_middle") \
    X(ExprTensorEnd, "tensor_end") \
    X(ExprTensorSingle, "tensor_single") \
    X(ExprDictValueStart, "dict_value_start") \
    X(ExprDictValueMiddle, "dict_value_middle") \
    X(ExprDictValueEnd, "dict_value_end") \
    X(ExprDictValueSingle, "dict_value_single") \
    X(ExprBracketAccessIndex, "bracket_access_index") \
    X(ExprCallArgStart, "call_arg_start") \
    X(ExprCallArgMiddle, "call_arg_middle") \
    X(ExprCallArgEnd, "call_arg_end") \
    X(ExprCallArgSingle, "call_arg_single") \
    X(ExprBinaryLhs, "binary_lhs") \
    X(ExprBinaryRhs, "binary_rhs") \
    X(ExprGrouping, "grouping") \
    X(ExprUnaryGrouping, "unary_grouping") \
    X(ExprAsCallTarget, "as_call_target") \
    X(ExprAsDotTarget, "as_dot_target") \
    X(ExprAsBracketTarget, "as_bracket_target") \
    X(ExprAsSliceTarget, "as_slice_target") \
    X(ExprSwitchCond, "switch_cond") \
    X(ExprSwitchCaseStart, "switch_case_start") \
    X(ExprSwitchCaseMiddle, "switch_case_middle") \
    X(ExprSwitchCaseEnd, "switch_case_end") \
    X(ExprSwitchCaseSingle, "switch_case_single") \
    X(ExprIfCond, "if_cond") \
    X(ExprIfThen, "if_then") \
    X(ExprIfElse, "if_else")

#define VALUASCRIPT_MODIFIER_CONTEXTS(X) \
    X(ModBeforeLetSingle, "modifier_before_let_single") \
    X(ModBeforeLetMultiple, "modifier_before_let_multiple") \
    X(ModBeforeLetMultipleWithInner, "modifier_before_let_multiple_with_inner") \
    X(ModBeforeLetMultipleWithBothInner, "modifier_before_let_multiple_with_both_inner") \
    X(ModAssignment, "assignment") \
    X(ModMultiAssignment, "multi_assignment") \
    X(ModImportStatement, "import_statement") \
    X(ModReturnStatement, "return_statement") \
    X(ModSwitchCase, "switch_case") \
    X(ModSwitchDefault, "switch_default") \
    X(ModFunctionDefinition, "function_definition") \
    X(ModFunctionParameter, "function_parameter") \
    X(ModFunctionParameterWithDefault, "function_parameter_with_default") \
    X(ModStructDefinition, "struct_definition") \
    X(ModStructField, "struct_field") \
    X(ModEnumDefinition, "enum_definition") \
    X(ModEnumCase, "enum_case") \
    X(ModTypealiasDefinition, "typealias_definition") \
    X(ModExtensionDefinition, "extension_definition") \
    X(ModDictItem, "dict_item")

#define VALUASCRIPT_TYPE_CONTEXTS(X) \
    X(TypeAssignmentTarget, "assignment_target") \
    X(TypeMultiAssignmentTarget1, "multi_assignment_target_1") \
    X(TypeMultiAssignmentTarget2, "multi_assignment_target_2") \
    X(TypeTypealiasTarget, "typealias_target") \
    X(TypeExtensionTarget, "extension_target") \
    X(TypeFunctionParamStart, "function_param_start") \
    X(TypeFunctionParamMiddle, "function_param_middle") \
    X(TypeFunctionParamEnd, "function_param_end") \
    X(TypeFunctionParamSingle, "function_param_single") \
    X(TypeFunctionReturnStart, "function_return_start") \
    X(TypeFunctionReturnMiddle, "function_return_middle") \
    X(TypeFunctionReturnEnd, "function_return_end") \
    X(TypeFunctionReturnSingle, "function_return_single") \
    X(TypeStructField, "struct_field") \
    X(TypeStructMultipleFields, "struct_multiple_fields") \
    X(TypeEnumUnderlyingType, "enum_underlying_type") \
    X(TypeTupleTypeStart, "tuple_type_start") \
    X(TypeTupleTypeMiddle, "tuple_type_middle") \
    X(TypeTupleTypeEnd, "tuple_type_end") \
    X(TypeGenericTypeStart, "generic_type_start") \
    X(TypeGenericTypeMiddle, "generic_type_middle") \
    X(TypeGenericTypeEnd, "generic_type_end") \
    X(TypeGenericTypeSingle, "generic_type_single")

#define VALUASCRIPT_IDENTIFIER_CONTEXTS(X) \
    X(IdLetTarget, "id_let_target") \
    X(IdMultiLetTarget1, "id_multi_let_target1") \
    X(IdMultiLetTarget2, "id_multi_let_target2") \
    X(IdReassignmentTarget, "id_reassignment_target") \
    X(IdFuncDefName, "id_func_def_name") \
    X(IdFuncParamName, "id_func_param_name") \
    X(IdStructDefName, "id_struct_def_name") \
    X(IdStructFieldNameStart, "id_struct_field_name_start") \
    X(IdStructFieldNameMiddle, "id_struct_field_name_middle") \
    X(IdStructFieldNameEnd, "id_struct_field_name_end") \
    X(IdStructFieldNameSingle, "id_struct_field_name_single") \
    X(IdEnumDefName, "id_enum_def_name") \
    X(IdEnumCaseStart, "id_enum_case_start") \
    X(IdEnumCaseMiddle, "id_enum_case_middle") \
    X(IdEnumCaseEnd, "id_enum_case_end") \
    X(IdEnumCaseSingle, "id_enum_case_single") \
    X(IdTypeAliasName, "id_typealias_name") \
    X(IdDirectiveName, "id_directive_name") \
    X(IdTypeAnnotation, "id_type_annotation") \
    X(IdDotAccessProperty, "id_dot_access_property") \
    X(IdDictKeyStart, "id_dict_key_start") \
    X(IdDictKeyMiddle, "id_dict_key_middle") \
    X(IdDictKeyEnd, "id_dict_key_end") \
    X(IdDictKeySingle, "id_dict_key_single") \
    X(IdSwitchCaseLabelStart, "id_switch_case_label_start") \
    X(IdSwitchCaseLabelMiddle, "id_switch_case_label_middle") \
    X(IdSwitchCaseLabelEnd, "id_switch_case_label_end") \
    X(IdSwitchCaseLabelSingle, "id_switch_case_label_single") \
    X(IdModifierName, "id_modifier_name") \
    X(IdModifierArgLabel, "id_modifier_arg_label") \
    X(IdCallArgLabel, "id_call_arg_label") \
    X(IdAsExpression, "id_as_expression") \
    X(IdStandaloneExpr, "id_standalone_expr")

#define VALUASCRIPT_ALL_CONTEXTS(X) \
    VALUASCRIPT_WRAPPER_CONTEXTS(X) \
    VALUASCRIPT_EXPRESSION_CONTEXTS(X) \
    VALUASCRIPT_MODIFIER_CONTEXTS(X) \
    VALUASCRIPT_TYPE_CONTEXTS(X) \
    VALUASCRIPT_IDENTIFIER_CONTEXTS(X)

#define VALUASCRIPT_DECLARE_CONTEXT_NAME(name, str) inline constexpr const char* name = str;
    VALUASCRIPT_ALL_CONTEXTS(VALUASCRIPT_DECLARE_CONTEXT_NAME)
#undef VALUASCRIPT_DECLARE_CONTEXT_NAME
}
