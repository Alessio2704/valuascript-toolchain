#include "errors/error_formatter.h"
#include "errors/valuascript_exception.h"

namespace valuascript::compiler {
    std::string_view get_error_template(const ErrorCode code) {
        switch (code) {
            // --- File Reader Errors ---
            case ErrorCode::FileNotFound:
                return "FileReaderStage Error: Cannot open file at path '{}'.";

            // --- Lexer Errors ---
            case ErrorCode::InvalidCharacter:
                return "Syntax Error: Invalid character '{}' found.";
            case ErrorCode::UnclosedString:
                return "Syntax Error: Unclosed string literal.";
            case ErrorCode::InvalidIdentifier:
                return "Syntax Error: Invalid identifier name.";
            case ErrorCode::UnterminatedDecimal:
                return "Syntax Error: Unterminated decimal number. Expected digits after '.'.";
            case ErrorCode::DecimalMissingLeadingZero:
                return "Syntax Error: Decimals must start with a leading zero (e.g., '0.5' instead of '.5').";

            // --- Parser Errors ---
            case ErrorCode::UnexpectedTopLevelToken:
                return "Syntax Error: Invalid syntax. Expected '#', 'let', 'var', 'enum', 'struct', 'func' or an identifier.";
            case ErrorCode::ExpectedImportToken:
                return "Expected 'import'.";
            case ErrorCode::MissingImportPathString:
                return "Syntax Error: Expected path after 'import'.";
            case ErrorCode::ExpectedHashToken:
                return "Expected '#'.";
            case ErrorCode::MissingDirectiveName:
                return "Syntax Error: Expected directive name after '#'.";
            case ErrorCode::ModifiersAttachedToInvalidDeclaration:
                return "Syntax Error: Modifiers must be attached to a declaration (let, var, func, struct, enum).";
            case ErrorCode::ExpectedModifierName:
                return "Syntax Error: Expected modifier name after '@'.";
            case ErrorCode::MissingArgumentNameInModifier:
                return "Expected argument name in modifier.";
            case ErrorCode::MissingColonAfterArgument:
                return "Expected ':' after argument name.";
            case ErrorCode::MissingCommaSeparatorForArgumentsInModifier:
                return "Syntax Error: Missing comma ',' between modifier arguments.";
            case ErrorCode::UnmatchedParenthesisAfterModifierArgs:
                return "Expected ')' after modifier arguments.";
            case ErrorCode::ExpectedStructToken:
                return "Expected 'struct'.";
            case ErrorCode::ExpectedStructName:
                return "Expected struct name.";
            case ErrorCode::ExpectedBraceInStructDefinition:
                return "Expected '{{' before struct body.";
            case ErrorCode::ExpectedStructFieldName:
                return "Expected field name.";
            case ErrorCode::ExpectedColonAfterStructFieldName:
                return "Expected ':'.";
            case ErrorCode::ExpectedCommaSeparatorInStruct:
                return "Syntax Error: Missing comma ',' between struct fields.";
            case ErrorCode::ExpectedRightBraceAfterStructBody:
                return "Expected '}}' after struct body.";
            case ErrorCode::ExpectedEnumToken:
                return "Expected 'enum' keyword.";
            case ErrorCode::ExpectedEnumName:
                return "Expected enum name.";
            case ErrorCode::ExpectedColonAfterEnumName:
                return "Expected ':' and underlying type after enum name.";
            case ErrorCode::ExpectedLeftBraceBeforeEnumBody:
                return "Expected '{{' before enum body.";
            case ErrorCode::ExpectedEnumCaseName:
                return "Expected enum case identifier.";
            case ErrorCode::ExpectedCommaSeparatorInEnum:
                return "Syntax Error: Missing comma ',' between enum cases.";
            case ErrorCode::ExpectedRightBraceAfterEnumBody:
                return "Expected '}}' after enum body.";
            case ErrorCode::ExpectedFuncToken:
                return "Expected 'func'.";
            case ErrorCode::MissingFunctionName:
                return "Syntax Error: Expected function name.";
            case ErrorCode::ExpectedLeftParenAfterFunctionName:
                return "Expected '(' after function name.";
            case ErrorCode::MissingParameterName:
                return "Syntax Error: Expected parameter name.";
            case ErrorCode::MissingColonAfterParameter:
                return "Expected ':' after parameter name.";
            case ErrorCode::ExpectedCommaSeparatorInParameterList:
                return "Syntax Error: Missing comma ',' between parameters.";
            case ErrorCode::ExpectedRightParenAfterParameters:
                return "Expected ')' after parameters.";
            case ErrorCode::MissingArrowInFunction:
                return "Expected '->' before return type.";
            case ErrorCode::MissingTypeAnnotationAfterArrow:
                return "Syntax Error: Expected at least one return type after '->'.";
            case ErrorCode::ExpectedCommaSeparatorInReturnTypeList:
                return "Syntax Error: Missing comma ',' between return types.";
            case ErrorCode::ExpectedLeftBraceBeforeFunctionBody:
                return "Expected '{{' before function body.";
            case ErrorCode::ExpectedRightBraceAfterFunctionBody:
                return "Expected '}}' after function body.";
            case ErrorCode::ExpectedCommaSeparatorInTupleType:
                return "Syntax Error: Missing comma ',' between tuple type elements.";
            case ErrorCode::UnmatchedParenthesisInTuple:
                return "Expected ')' after tuple type elements.";
            case ErrorCode::MissingTypeAnnotation:
                return "Expected a type name.";
            case ErrorCode::ExpectedCommaSeparatorInGenericArgs:
                return "Syntax Error: Missing comma ',' between generic type arguments.";
            case ErrorCode::EmptyGenericTypeAnnotation:
                return "Syntax Error: Expected at least one generic type argument inside '<>'.";
            case ErrorCode::UnmatchedBracketAfterGenericArgs:
                return "Expected '>' after generic type arguments.";
            case ErrorCode::ModifiersOnNonVariableDeclaration:
                return "Syntax Error: Modifiers can only be attached to variable declarations.";
            case ErrorCode::ExpectedLetOrVarToken:
                return "Expected 'let' or 'var'.";
            case ErrorCode::ReservedKeywordAsIdentifier:
                return "Syntax Error: Cannot use a reserved keyword as a variable name.";
            case ErrorCode::ExpectedCommaInMultiAssignment:
                return "Syntax Error: Missing comma ',' between variable names.";
            case ErrorCode::IncompleteAssignment:
                return "Syntax Error: Incomplete assignment. Expected '='.";
            case ErrorCode::MissingValueAfterEquals:
                return "Syntax Error: Missing value after '='.";
            case ErrorCode::MultiReassignmentNotSupported:
                return "Syntax Error: Multiple reassignment is not supported. Reassign variables individually.";
            case ErrorCode::InvalidLeftSideExpressionInReassignment:
                return "Syntax Error: Invalid assignment target. You can only assign to variables, properties, or indices.";
            case ErrorCode::InvalidStandaloneStatement:
                return "Syntax Error: Invalid statement. Expected an assignment, reassignment, or function call.";
            case ErrorCode::ChainingNotAllowedForComparisonOperations:
                return "Syntax Error: Chaining comparison operators is not allowed.";
            case ErrorCode::MissingOperatorOrArgumentName:
                return "Syntax Error: Missing operator (like '*') before '(', or expected argument name.";
            case ErrorCode::ExpectedArgumentNameOrClosingParen:
                return "Syntax Error: Expected an argument name (e.g., 'name: value') or a closing ')'.";
            case ErrorCode::MissingArgumentNameInFunctionCall:
                return "Expected argument name in function call.";
            case ErrorCode::MissingCommaSeparatorForArgumentsInFunctionCall:
                return "Syntax Error: Missing comma ',' between function arguments.";
            case ErrorCode::TrailingCommaInFunctionCall:
                return "Syntax Error: Trailing comma in function call arguments.";
            case ErrorCode::ExpectedRightParenAfterArguments:
                return "Expected ')' after arguments.";
            case ErrorCode::MissingOperatorOrExpectedColonOrBracketInTensor:
                return "Syntax Error: Missing operator or expected ':' or ']' in tensor access.";
            case ErrorCode::UnexpectedCommaInBracketAccess:
                return "Syntax Error: Unexpected ',' inside bracket access. If you meant to write a second tensor, you are missing an operator (like '+') between them.";
            case ErrorCode::EmptyBracketAccess:
                return "Expected an index or slice inside '[]'.";
            case ErrorCode::UnmatchedBracketAfterTensorIndex:
                return "Expected ']' after tensor index.";
            case ErrorCode::ExpectedPropertyName:
                return "Expected property name after '.'.";
            case ErrorCode::InvalidExpression:
                return "Syntax Error: Expected an expression.";
            case ErrorCode::SingleElementTuplesNotAllowed:
                return "Syntax Error: Trailing commas and 1-element tuples are not allowed.";
            case ErrorCode::TrailingCommaInTuple:
                return "Syntax Error: Trailing comma in tuple.";
            case ErrorCode::ExpectedRightParenAfterTupleElements:
                return "Expected ')' after tuple elements.";
            case ErrorCode::MissingOperatorInsideGrouping:
                return "Syntax Error: Missing operator between expressions inside grouping.";
            case ErrorCode::ExpectedRightParenAfterExpression:
                return "Expected ')' after expression.";
            case ErrorCode::UnmatchedBracketAfterVectorElements:
                return "Expected ']' after vector elements.";
            case ErrorCode::ExpectedDictionaryKey:
                return "Expected key in dictionary.";
            case ErrorCode::ExpectedColonAfterDictionaryKey:
                return "Expected ':' after dictionary key.";
            case ErrorCode::ExpectedCommaSeparatorInDictionaryLiteral:
                return "Syntax Error: Missing comma ',' between dictionary fields.";
            case ErrorCode::UnmatchedBraceInDictionaryLiteral:
                return "Expected '}}' after dictionary literal.";
            case ErrorCode::MissingThenToken:
                return "Expected 'then'.";
            case ErrorCode::MissingElseToken:
                return "Expected 'else'.";
            case ErrorCode::ExpectedLeftParenAfterSwitch:
                return "Expected '(' after 'switch'.";
            case ErrorCode::ExpectedRightParenAfterSwitchTarget:
                return "Expected ')' after switch target.";
            case ErrorCode::ExpectedLeftBraceBeforeSwitchBody:
                return "Expected '{{' before switch body.";
            case ErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier:
                return "Expected '->' before case result.";
            case ErrorCode::MissingOperatorInSwitchCaseResult:
                return "Syntax Error: Missing operator between expressions in switch case result.";
            case ErrorCode::CaseOrDefaultMissingInSwitchAfterResult:
                return "Syntax Error: Expected 'case', 'default', or '}' after case result.";
            case ErrorCode::ExpectedEnumCaseNameAfterCase:
                return "Expected enum case identifier after 'case'.";
            case ErrorCode::ExpectedCommaBetweenCaseIdentifiers:
                return "Syntax Error: Missing comma ',' between case identifiers.";
            case ErrorCode::MultipleDefaultCasesInSwitch:
                return "Syntax Error: A switch expression can only have one 'default' case.";
            case ErrorCode::ExpectedCaseOrDefaultInsideSwitchBody:
                return "Syntax Error: Expected 'case' or 'default' inside switch body.";
            case ErrorCode::ExpectedRightBraceAfterSwitchBody:
                return "Expected '}}' after switch body.";
            case ErrorCode::MissingOperator:
                return "Syntax Error: Missing operator between expressions.";
            case ErrorCode::TrailingCommaInList:
                return "Syntax Error: Trailing comma in list.";
            case ErrorCode::MissingCommaOrOperatorBetweenExpressions:
                return "Syntax Error: Missing comma ',' or operator between expressions.";
            case ErrorCode::TrailingComma:
                return "Syntax Error: Trailing comma.";
            case ErrorCode::MissingCommaBetweenFields:
                return "Syntax Error: Missing comma ',' between fields.";
            case ErrorCode::TopLevelDeclarationInsideFunction:
                return "Syntax Error: Top level declaration inside function.";

            // --- Import Resolver Errors ---
            case ErrorCode::CircularImportDetected:
                return "Import Error: Circular import detected involving '{}'.";
            case ErrorCode::ImportFileNotFound:
                return "Import Error: Cannot find module '{}'.";
        }

        return "An unknown error occurred.";
    }
}
