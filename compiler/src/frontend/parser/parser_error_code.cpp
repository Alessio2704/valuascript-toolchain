#include "parser_error_code.h"

namespace valuascript::compiler
{
    std::string_view get_error_template(ParserErrorCode code)
    {
        switch (code)
        {
        case ParserErrorCode::UnexpectedTopLevelToken: return
                "Syntax Error: Invalid syntax. Expected '#', 'let', 'enum', 'struct', 'func' or an identifier.";
        case ParserErrorCode::ExpectedImportToken: return "Expected 'import'.";
        case ParserErrorCode::MissingImportPathString: return "Syntax Error: Expected path after 'import'.";
        case ParserErrorCode::ExpectedHashToken: return "Expected '#'.";
        case ParserErrorCode::MissingDirectiveName: return "Syntax Error: Expected directive name after '#'.";
        case ParserErrorCode::ModifiersAttachedToInvalidDeclaration: return
                "Syntax Error: Modifiers must be attached to a declaration";
        case ParserErrorCode::ExpectedModifierName: return "Syntax Error: Expected modifier name after '@'.";
        case ParserErrorCode::MissingArgumentNameInModifier: return "Expected argument name in modifier.";
        case ParserErrorCode::MissingColonAfterArgument: return "Expected ':' after argument name.";
        case ParserErrorCode::MissingCommaSeparatorForArgumentsInModifier: return
                "Syntax Error: Missing comma ',' between modifier arguments.";
        case ParserErrorCode::UnmatchedParenthesisAfterModifierArgs: return "Expected ')' after modifier arguments.";
        case ParserErrorCode::ExpectedStructToken: return "Expected 'struct'.";
        case ParserErrorCode::ExpectedStructName: return "Expected struct name.";
        case ParserErrorCode::ExpectedBraceInStructDefinition: return "Expected '{{' before struct body.";
        case ParserErrorCode::ExpectedStructFieldName: return "Expected field name.";
        case ParserErrorCode::ExpectedColonAfterStructFieldName: return "Expected ':'.";
        case ParserErrorCode::ExpectedCommaSeparatorInStruct: return
                "Syntax Error: Missing comma ',' between struct fields.";
        case ParserErrorCode::ExpectedRightBraceAfterStructBody: return "Expected '}}' after struct body.";
        case ParserErrorCode::ExpectedEnumToken: return "Expected 'enum' keyword.";
        case ParserErrorCode::ExpectedEnumName: return "Expected enum name.";
        case ParserErrorCode::ExpectedColonAfterEnumName: return "Expected ':' and underlying type after enum name.";
        case ParserErrorCode::ExpectedLeftBraceBeforeEnumBody: return "Expected '{{' before enum body.";
        case ParserErrorCode::ExpectedEnumCaseName: return "Expected enum case identifier.";
        case ParserErrorCode::ExpectedCommaSeparatorInEnum: return
                "Syntax Error: Missing comma ',' between enum cases.";
        case ParserErrorCode::ExpectedRightBraceAfterEnumBody: return "Expected '}}' after enum body.";
        case ParserErrorCode::ExpectedFuncToken: return "Expected 'func'.";
        case ParserErrorCode::MissingFunctionName: return "Syntax Error: Expected function name.";
        case ParserErrorCode::ExpectedLeftParenAfterFunctionName: return "Expected '(' after function name.";
        case ParserErrorCode::MissingParameterName: return "Syntax Error: Expected parameter name.";
        case ParserErrorCode::MissingColonAfterParameter: return "Expected ':' after parameter name.";
        case ParserErrorCode::MissingDefaultParameterValue: return
                "Syntax Error: Expected an expression for the default parameter value.";
        case ParserErrorCode::NonDefaultParameterAfterDefault: return
                "Syntax Error: Non-default parameters cannot follow default parameters.";
        case ParserErrorCode::ExpectedCommaSeparatorInParameterList: return
                "Syntax Error: Missing comma ',' between parameters.";
        case ParserErrorCode::ExpectedRightParenAfterParameters: return "Expected ')' after parameters.";
        case ParserErrorCode::MissingArrowInFunction: return "Expected '->' before return type.";
        case ParserErrorCode::MissingTypeAnnotationAfterArrow: return
                "Syntax Error: Expected at least one return type after '->'.";
        case ParserErrorCode::ExpectedCommaSeparatorInReturnTypeList: return
                "Syntax Error: Missing comma ',' between return types.";
        case ParserErrorCode::ExpectedLeftBraceBeforeFunctionBody: return "Expected '{{' before function body.";
        case ParserErrorCode::ExpectedRightBraceAfterFunctionBody: return "Expected '}}' after function body.";
        case ParserErrorCode::ExpectedCommaSeparatorInTupleType: return
                "Syntax Error: Missing comma ',' between tuple type elements.";
        case ParserErrorCode::UnmatchedParenthesisInTuple: return "Expected ')' after tuple type elements.";
        case ParserErrorCode::MissingTypeAnnotation: return "Expected a type name.";
        case ParserErrorCode::ExpectedCommaSeparatorInGenericArgs: return
                "Syntax Error: Missing comma ',' between generic type arguments.";
        case ParserErrorCode::EmptyGenericTypeAnnotation: return
                "Syntax Error: Expected at least one generic type argument inside '<>'.";
        case ParserErrorCode::UnmatchedBracketAfterGenericArgs: return "Expected '>' after generic type arguments.";
        case ParserErrorCode::ExpectedLetToken: return "Expected 'let' keyword for declaration.";
        case ParserErrorCode::ExpectedIdentifier: return "Syntax Error: Expected name identifier.";
        case ParserErrorCode::ExpectedColon: return "Syntax Error: Expected colon ':'.";
        case ParserErrorCode::ReservedKeywordAsIdentifier: return
                "Syntax Error: Cannot use a reserved keyword as an identifier.";
        case ParserErrorCode::ExpectedCommaInMultiAssignment: return
                "Syntax Error: Missing comma ',' between variable names.";
        case ParserErrorCode::IncompleteAssignment: return "Syntax Error: Incomplete assignment. Expected '='.";
        case ParserErrorCode::MissingValueAfterEquals: return "Syntax Error: Missing value after '='.";
        case ParserErrorCode::MultiReassignmentNotSupported: return
                "Syntax Error: Multiple reassignment is not supported. Reassign variables individually.";
        case ParserErrorCode::InvalidLeftSideExpressionInReassignment: return
                "Syntax Error: Invalid assignment target. You can only assign to variables, properties, or indices.";
        case ParserErrorCode::InvalidStandaloneStatement: return
                "Syntax Error: Invalid statement. Expected an assignment, reassignment, or function call.";
        case ParserErrorCode::ChainingNotAllowedForComparisonOperations: return
                "Syntax Error: Chaining comparison operators is not allowed.";
        case ParserErrorCode::MissingOperatorOrArgumentName: return
                "Syntax Error: Missing operator (like '*') before '(', or expected argument name.";
        case ParserErrorCode::ExpectedArgumentNameOrClosingParen: return
                "Syntax Error: Expected an argument name (e.g., 'name: value') or a closing ')'.";
        case ParserErrorCode::MissingArgumentNameInFunctionCall: return "Expected argument name in function call.";
        case ParserErrorCode::MissingCommaSeparatorForArgumentsInFunctionCall: return
                "Syntax Error: Missing comma ',' between function arguments.";
        case ParserErrorCode::TrailingCommaInFunctionCall: return
                "Syntax Error: Trailing comma in function call arguments.";
        case ParserErrorCode::TrailingCommaInModifier: return "Syntax Error: Trailing comma in modifier arguments.";
        case ParserErrorCode::ExpectedRightParenAfterArguments: return "Expected ')' after arguments.";
        case ParserErrorCode::UnexpectedCommaInBracketAccess: return
                "Syntax Error: Unexpected ',' inside bracket access. If you meant to write a second tensor, you are missing an operator (like '+') between them.";
        case ParserErrorCode::EmptyBracketAccess: return "Expected an index or slice inside '[]'.";
        case ParserErrorCode::UnmatchedBracketAfterTensorIndex: return "Expected ']' after tensor index.";
        case ParserErrorCode::ExpectedPropertyName: return "Expected property name after '.'.";
        case ParserErrorCode::InvalidExpression: return "Syntax Error: Invalid expression.";
        case ParserErrorCode::SingleElementTuplesNotAllowed: return
                "Syntax Error: Trailing commas and 1-element tuples are not allowed.";
        case ParserErrorCode::TrailingCommaInTuple: return "Syntax Error: Trailing comma in tuple.";
        case ParserErrorCode::ExpectedRightParenAfterTupleElements: return "Expected ')' after tuple elements.";
        case ParserErrorCode::ExpectedRightParenAfterExpression: return "Expected ')' after expression.";
        case ParserErrorCode::UnmatchedBracketAfterTensorElements: return "Expected ']' after vector elements.";
        case ParserErrorCode::ExpectedDictionaryKey: return "Expected key in dictionary.";
        case ParserErrorCode::ExpectedColonAfterDictionaryKey: return "Expected ':' after dictionary key.";
        case ParserErrorCode::ExpectedCommaSeparatorInDictionaryLiteral: return
                "Syntax Error: Missing comma ',' between dictionary fields.";
        case ParserErrorCode::UnmatchedBraceInDictionaryLiteral: return "Expected '}}' after dictionary literal.";
        case ParserErrorCode::MissingThenToken: return "Expected 'then'.";
        case ParserErrorCode::MissingElseToken: return "Expected 'else'.";
        case ParserErrorCode::ExpectedLeftParenAfterSwitch: return "Expected '(' after 'switch'.";
        case ParserErrorCode::ExpectedRightParenAfterSwitchTarget: return "Expected ')' after switch target.";
        case ParserErrorCode::ExpectedLeftBraceBeforeSwitchBody: return "Expected '{{' before switch body.";
        case ParserErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier: return "Expected '->' before case result.";
        case ParserErrorCode::CaseOrDefaultMissingInSwitchAfterResult: return
                "Syntax Error: Expected 'case', 'default', or '}}' after case result.";
        case ParserErrorCode::ExpectedEnumCaseNameAfterCase: return "Expected enum case identifier after 'case'.";
        case ParserErrorCode::ExpectedCommaBetweenCaseIdentifiers: return
                "Syntax Error: Missing comma ',' between case identifiers.";
        case ParserErrorCode::MultipleDefaultCasesInSwitch: return
                "Syntax Error: A switch expression can only have one 'default' case.";
        case ParserErrorCode::ExpectedCaseOrDefaultInsideSwitchBody: return
                "Syntax Error: Expected 'case' or 'default' inside switch body.";
        case ParserErrorCode::ExpectedRightBraceAfterSwitchBody: return "Expected '}}' after switch body.";
        case ParserErrorCode::MissingOperator: return "Syntax Error: Missing operator between expressions.";
        case ParserErrorCode::TrailingCommaInList: return "Syntax Error: Trailing comma in list.";
        case ParserErrorCode::MissingCommaOrOperatorBetweenExpressions: return
                "Syntax Error: Missing comma ',' or operator between expressions.";
        case ParserErrorCode::TrailingComma: return "Syntax Error: Trailing comma.";
        case ParserErrorCode::TrailingCommaInGenericArgument: return
                "Syntax Error: Trailing comma in generic arguments.";
        case ParserErrorCode::MissingCommaBetweenFields: return "Syntax Error: Missing comma ',' between fields.";
        case ParserErrorCode::TopLevelDeclarationNotAllowedHere: return
                "Syntax Error: Top level declaration not allowed here.";
        case ParserErrorCode::ReturnUsedInToplevel: return "Syntax Error: 'return' keyword used at top level.";
        case ParserErrorCode::ReturnNotAllowedInExtension: return "Syntax Error: 'return' statement is not allowed inside an extension body.";
        case ParserErrorCode::ExpectedTypeAliasToken: return "Expected 'typealias' keyword.";
        case ParserErrorCode::ExpectedTypeAliasName: return "Expected type alias name.";
        case ParserErrorCode::ExpectedAssignAfterTypeAliasName: return "Expected '=' after type alias name.";
        case ParserErrorCode::ExpectedExtensionToken: return "Expected 'extension' keyword.";
        case ParserErrorCode::ExpectedLeftBraceBeforeExtensionBody: return "Expected '{{' before extension body.";
        case ParserErrorCode::ExpectedRightBraceAfterExtensionBody: return "Expected '}}' after extension body.";
        case ParserErrorCode::ImportNotAllowedInExtension: return "Syntax Error: 'import' is not allowed inside an extension.";
        case ParserErrorCode::DirectiveNotAllowedInExtension: return "Syntax Error: Directives are not allowed inside an extension.";
        }
        return "Unknown Parser Error";
    }
}
