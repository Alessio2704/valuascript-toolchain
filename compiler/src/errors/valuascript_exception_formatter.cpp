#include "errors/error_formatter.h"
#include "errors/valuascript_exception.h"

namespace valuascript::compiler {
    std::string_view get_error_template(const ValuascriptErrorCode code) {
        switch (code) {
            // --- File Reader Errors ---
            case ValuascriptErrorCode::FileNotFound:
                return "FileReaderStage Error: Cannot open file at path '{}'.";

            // --- Lexer Errors ---
            case ValuascriptErrorCode::InvalidCharacter:
                return "Syntax Error: Invalid character '{}' found.";
            case ValuascriptErrorCode::UnclosedString:
                return "Syntax Error: Unclosed string literal.";
            case ValuascriptErrorCode::InvalidIdentifier:
                return "Syntax Error: Invalid identifier name.";
            case ValuascriptErrorCode::UnterminatedDecimal:
                return "Syntax Error: Unterminated decimal number. Expected digits after '.'.";
            case ValuascriptErrorCode::DecimalMissingLeadingZero:
                return "Syntax Error: Decimals must start with a leading zero (e.g., '0.5' instead of '.5').";
            case ValuascriptErrorCode::TrailingSeparatorInNumberLiteral:
                return "Syntax Error: Trailing separator '_' in number literal (remove it or complete with digits after separator).";

            // --- Parser Errors ---
            case ValuascriptErrorCode::UnexpectedTopLevelToken:
                return "Syntax Error: Invalid syntax. Expected '#', 'let', 'var', 'enum', 'struct', 'func' or an identifier.";
            case ValuascriptErrorCode::ExpectedImportToken:
                return "Expected 'import'.";
            case ValuascriptErrorCode::MissingImportPathString:
                return "Syntax Error: Expected path after 'import'.";
            case ValuascriptErrorCode::ExpectedHashToken:
                return "Expected '#'.";
            case ValuascriptErrorCode::MissingDirectiveName:
                return "Syntax Error: Expected directive name after '#'.";
            case ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration:
                return "Syntax Error: Modifiers must be attached to a declaration (let, var, func, struct, enum).";
            case ValuascriptErrorCode::ModifiersAttachedToMultiAssignementSingleElements:
                return "Syntax Error: Modifiers for multi assignment must be attached before the overall statement.";
            case ValuascriptErrorCode::ExpectedModifierName:
                return "Syntax Error: Expected modifier name after '@'.";
            case ValuascriptErrorCode::MissingArgumentNameInModifier:
                return "Expected argument name in modifier.";
            case ValuascriptErrorCode::MissingColonAfterArgument:
                return "Expected ':' after argument name.";
            case ValuascriptErrorCode::MissingCommaSeparatorForArgumentsInModifier:
                return "Syntax Error: Missing comma ',' between modifier arguments.";
            case ValuascriptErrorCode::UnmatchedParenthesisAfterModifierArgs:
                return "Expected ')' after modifier arguments.";
            case ValuascriptErrorCode::ExpectedStructToken:
                return "Expected 'struct'.";
            case ValuascriptErrorCode::ExpectedStructName:
                return "Expected struct name.";
            case ValuascriptErrorCode::ExpectedBraceInStructDefinition:
                return "Expected '{{' before struct body.";
            case ValuascriptErrorCode::ExpectedStructFieldName:
                return "Expected field name.";
            case ValuascriptErrorCode::ExpectedColonAfterStructFieldName:
                return "Expected ':'.";
            case ValuascriptErrorCode::ExpectedCommaSeparatorInStruct:
                return "Syntax Error: Missing comma ',' between struct fields.";
            case ValuascriptErrorCode::ExpectedRightBraceAfterStructBody:
                return "Expected '}}' after struct body.";
            case ValuascriptErrorCode::ExpectedEnumToken:
                return "Expected 'enum' keyword.";
            case ValuascriptErrorCode::ExpectedEnumName:
                return "Expected enum name.";
            case ValuascriptErrorCode::ExpectedColonAfterEnumName:
                return "Expected ':' and underlying type after enum name.";
            case ValuascriptErrorCode::ExpectedLeftBraceBeforeEnumBody:
                return "Expected '{{' before enum body.";
            case ValuascriptErrorCode::ExpectedEnumCaseName:
                return "Expected enum case identifier.";
            case ValuascriptErrorCode::ExpectedCommaSeparatorInEnum:
                return "Syntax Error: Missing comma ',' between enum cases.";
            case ValuascriptErrorCode::ExpectedRightBraceAfterEnumBody:
                return "Expected '}}' after enum body.";
            case ValuascriptErrorCode::ExpectedFuncToken:
                return "Expected 'func'.";
            case ValuascriptErrorCode::MissingFunctionName:
                return "Syntax Error: Expected function name.";
            case ValuascriptErrorCode::ExpectedLeftParenAfterFunctionName:
                return "Expected '(' after function name.";
            case ValuascriptErrorCode::MissingParameterName:
                return "Syntax Error: Expected parameter name.";
            case ValuascriptErrorCode::MissingColonAfterParameter:
                return "Expected ':' after parameter name.";
            case ValuascriptErrorCode::ExpectedCommaSeparatorInParameterList:
                return "Syntax Error: Missing comma ',' between parameters.";
            case ValuascriptErrorCode::ExpectedRightParenAfterParameters:
                return "Expected ')' after parameters.";
            case ValuascriptErrorCode::MissingArrowInFunction:
                return "Expected '->' before return type.";
            case ValuascriptErrorCode::MissingTypeAnnotationAfterArrow:
                return "Syntax Error: Expected at least one return type after '->'.";
            case ValuascriptErrorCode::ExpectedCommaSeparatorInReturnTypeList:
                return "Syntax Error: Missing comma ',' between return types.";
            case ValuascriptErrorCode::ExpectedLeftBraceBeforeFunctionBody:
                return "Expected '{{' before function body.";
            case ValuascriptErrorCode::ExpectedRightBraceAfterFunctionBody:
                return "Expected '}}' after function body.";
            case ValuascriptErrorCode::ExpectedCommaSeparatorInTupleType:
                return "Syntax Error: Missing comma ',' between tuple type elements.";
            case ValuascriptErrorCode::UnmatchedParenthesisInTuple:
                return "Expected ')' after tuple type elements.";
            case ValuascriptErrorCode::MissingTypeAnnotation:
                return "Expected a type name.";
            case ValuascriptErrorCode::ExpectedCommaSeparatorInGenericArgs:
                return "Syntax Error: Missing comma ',' between generic type arguments.";
            case ValuascriptErrorCode::EmptyGenericTypeAnnotation:
                return "Syntax Error: Expected at least one generic type argument inside '<>'.";
            case ValuascriptErrorCode::UnmatchedBracketAfterGenericArgs:
                return "Expected '>' after generic type arguments.";
            case ValuascriptErrorCode::TrailingCommaInGenericArgument:
                return "Syntax Error: Trailing comma in generic arguments.";
            case ValuascriptErrorCode::ExpectedLetOrVarToken:
                return "Expected 'let' or 'var'.";
            case ValuascriptErrorCode::ReservedKeywordAsIdentifier:
                return "Syntax Error: Cannot use a reserved keyword as an identifier.";
            case ValuascriptErrorCode::ExpectedCommaInMultiAssignment:
                return "Syntax Error: Missing comma ',' between variable names.";
            case ValuascriptErrorCode::IncompleteAssignment:
                return "Syntax Error: Incomplete assignment. Expected '='.";
            case ValuascriptErrorCode::MissingValueAfterEquals:
                return "Syntax Error: Missing value after '='.";
            case ValuascriptErrorCode::MultiReassignmentNotSupported:
                return "Syntax Error: Multiple reassignment is not supported. Reassign variables individually.";
            case ValuascriptErrorCode::InvalidLeftSideExpressionInReassignment:
                return "Syntax Error: Invalid assignment target. You can only assign to variables, properties, or indices.";
            case ValuascriptErrorCode::InvalidStandaloneStatement:
                return "Syntax Error: Invalid statement. Expected an assignment, reassignment, or function call.";
            case ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations:
                return "Syntax Error: Chaining comparison operators is not allowed.";
            case ValuascriptErrorCode::MissingOperatorOrArgumentName:
                return "Syntax Error: Missing operator (like '*') before '(', or expected argument name.";
            case ValuascriptErrorCode::ExpectedArgumentNameOrClosingParen:
                return "Syntax Error: Expected an argument name (e.g., 'name: value') or a closing ')'.";
            case ValuascriptErrorCode::MissingArgumentNameInFunctionCall:
                return "Expected argument name in function call.";
            case ValuascriptErrorCode::MissingCommaSeparatorForArgumentsInFunctionCall:
                return "Syntax Error: Missing comma ',' between function arguments.";
            case ValuascriptErrorCode::TrailingCommaInFunctionCall:
                return "Syntax Error: Trailing comma in function call arguments.";
            case ValuascriptErrorCode::TrailingCommaInModifier:
                return "Syntax Error: Trailing comma in modifier arguments.";
            case ValuascriptErrorCode::ExpectedRightParenAfterArguments:
                return "Expected ')' after arguments.";
            case ValuascriptErrorCode::MissingOperatorOrExpectedColonOrBracketInTensor:
                return "Syntax Error: Missing operator or expected ':' or ']' in tensor access.";
            case ValuascriptErrorCode::UnexpectedCommaInBracketAccess:
                return "Syntax Error: Unexpected ',' inside bracket access. If you meant to write a second tensor, you are missing an operator (like '+') between them.";
            case ValuascriptErrorCode::EmptyBracketAccess:
                return "Expected an index or slice inside '[]'.";
            case ValuascriptErrorCode::UnmatchedBracketAfterTensorIndex:
                return "Expected ']' after tensor index.";
            case ValuascriptErrorCode::ExpectedPropertyName:
                return "Expected property name after '.'.";
            case ValuascriptErrorCode::InvalidExpression:
                return "Syntax Error: Expected an expression.";
            case ValuascriptErrorCode::SingleElementTuplesNotAllowed:
                return "Syntax Error: Trailing commas and 1-element tuples are not allowed.";
            case ValuascriptErrorCode::TrailingCommaInTuple:
                return "Syntax Error: Trailing comma in tuple.";
            case ValuascriptErrorCode::ExpectedRightParenAfterTupleElements:
                return "Expected ')' after tuple elements.";
            case ValuascriptErrorCode::MissingOperatorInsideGrouping:
                return "Syntax Error: Missing operator between expressions inside grouping.";
            case ValuascriptErrorCode::ExpectedRightParenAfterExpression:
                return "Expected ')' after expression.";
            case ValuascriptErrorCode::UnmatchedBracketAfterTensorElements:
                return "Expected ']' after vector elements.";
            case ValuascriptErrorCode::ExpectedDictionaryKey:
                return "Expected key in dictionary.";
            case ValuascriptErrorCode::ExpectedColonAfterDictionaryKey:
                return "Expected ':' after dictionary key.";
            case ValuascriptErrorCode::ExpectedCommaSeparatorInDictionaryLiteral:
                return "Syntax Error: Missing comma ',' between dictionary fields.";
            case ValuascriptErrorCode::UnmatchedBraceInDictionaryLiteral:
                return "Expected '}}' after dictionary literal.";
            case ValuascriptErrorCode::MissingThenToken:
                return "Expected 'then'.";
            case ValuascriptErrorCode::MissingElseToken:
                return "Expected 'else'.";
            case ValuascriptErrorCode::ExpectedLeftParenAfterSwitch:
                return "Expected '(' after 'switch'.";
            case ValuascriptErrorCode::ExpectedRightParenAfterSwitchTarget:
                return "Expected ')' after switch target.";
            case ValuascriptErrorCode::ExpectedLeftBraceBeforeSwitchBody:
                return "Expected '{{' before switch body.";
            case ValuascriptErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier:
                return "Expected '->' before case result.";
            case ValuascriptErrorCode::MissingOperatorInSwitchCaseResult:
                return "Syntax Error: Missing operator between expressions in switch case result.";
            case ValuascriptErrorCode::CaseOrDefaultMissingInSwitchAfterResult:
                return "Syntax Error: Expected 'case', 'default', or '}}' after case result.";
            case ValuascriptErrorCode::ExpectedEnumCaseNameAfterCase:
                return "Expected enum case identifier after 'case'.";
            case ValuascriptErrorCode::ExpectedCommaBetweenCaseIdentifiers:
                return "Syntax Error: Missing comma ',' between case identifiers.";
            case ValuascriptErrorCode::MultipleDefaultCasesInSwitch:
                return "Syntax Error: A switch expression can only have one 'default' case.";
            case ValuascriptErrorCode::ExpectedCaseOrDefaultInsideSwitchBody:
                return "Syntax Error: Expected 'case' or 'default' inside switch body.";
            case ValuascriptErrorCode::ExpectedRightBraceAfterSwitchBody:
                return "Expected '}}' after switch body.";
            case ValuascriptErrorCode::MissingOperator:
                return "Syntax Error: Missing operator between expressions.";
            case ValuascriptErrorCode::TrailingCommaInList:
                return "Syntax Error: Trailing comma in list.";
            case ValuascriptErrorCode::MissingCommaOrOperatorBetweenExpressions:
                return "Syntax Error: Missing comma ',' or operator between expressions.";
            case ValuascriptErrorCode::TrailingComma:
                return "Syntax Error: Trailing comma.";
            case ValuascriptErrorCode::MissingCommaBetweenFields:
                return "Syntax Error: Missing comma ',' between fields.";
            case ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere:
                return "Syntax Error: Top level declaration not allowed here.";

            // --- Import Resolver Errors ---
            case ValuascriptErrorCode::CircularImportDetected:
                return "Import Error: Circular import detected involving '{}'.";
            case ValuascriptErrorCode::ImportFileNotFound:
                return "Import Error: Cannot find module '{}'.";
        }

        return "An unknown error occurred.";
    }
}
