#pragma once
#include <string>
#include <utility>

#include "stages/frontend/parser/ast.h"

namespace valuascript::compiler {
    enum class ErrorCategory { File, Lexical, Syntax, Semantic, Import, Internal };

    enum class ErrorCode {
        // --- File Reader Errors ---
        FileNotFound,

        // --- Lexer Errors ---
        InvalidCharacter,
        UnclosedString,
        InvalidIdentifier,
        DecimalMissingLeadingZero,
        UnterminatedDecimal,

        // --- Parser Errors ---
        UnexpectedTopLevelToken,
        ExpectedImportToken,
        MissingImportPathString,
        ExpectedHashToken,
        MissingDirectiveName,
        ModifiersAttachedToInvalidDeclaration,
        ExpectedModifierName,
        MissingArgumentNameInModifier,
        MissingColonAfterArgument,
        MissingCommaSeparatorForArgumentsInModifier,
        UnmatchedParenthesisAfterModifierArgs,
        ExpectedStructToken,
        ExpectedStructName,
        ExpectedBraceInStructDefinition,
        ExpectedStructFieldName,
        ExpectedColonAfterStructFieldName,
        ExpectedCommaSeparatorInStruct,
        ExpectedRightBraceAfterStructBody,
        ExpectedEnumToken,
        ExpectedEnumName,
        ExpectedColonAfterEnumName,
        ExpectedLeftBraceBeforeEnumBody,
        ExpectedEnumCaseName,
        ExpectedCommaSeparatorInEnum,
        ExpectedRightBraceAfterEnumBody,
        ExpectedFuncToken,
        MissingFunctionName,
        ExpectedLeftParenAfterFunctionName,
        MissingParameterName,
        MissingColonAfterParameter,
        ExpectedCommaSeparatorInParameterList,
        ExpectedRightParenAfterParameters,
        MissingArrowInFunction,
        MissingTypeAnnotationAfterArrow,
        ExpectedCommaSeparatorInReturnTypeList,
        ExpectedLeftBraceBeforeFunctionBody,
        ExpectedRightBraceAfterFunctionBody,
        ExpectedCommaSeparatorInTupleType,
        UnmatchedParenthesisInTuple,
        MissingTypeAnnotation,
        ExpectedCommaSeparatorInGenericArgs,
        EmptyGenericTypeAnnotation,
        UnmatchedBracketAfterGenericArgs,
        ModifiersOnNonVariableDeclaration,
        ExpectedLetOrVarToken,
        ReservedKeywordAsIdentifier,
        ExpectedCommaInMultiAssignment,
        IncompleteAssignment,
        MissingValueAfterEquals,
        MultiReassignmentNotSupported,
        InvalidLeftSideExpressionInReassignment,
        InvalidStandaloneStatement,
        ChainingNotAllowedForComparisonOperations,
        MissingOperatorOrArgumentName,
        ExpectedArgumentNameOrClosingParen,
        MissingArgumentNameInFunctionCall,
        MissingCommaSeparatorForArgumentsInFunctionCall,
        TrailingCommaInFunctionCall,
        ExpectedRightParenAfterArguments,
        MissingOperatorOrExpectedColonOrBracketInTensor,
        UnexpectedCommaInBracketAccess,
        EmptyBracketAccess,
        UnmatchedBracketAfterTensorIndex,
        ExpectedPropertyName,
        InvalidExpression,
        SingleElementTuplesNotAllowed,
        TrailingCommaInTuple,
        ExpectedRightParenAfterTupleElements,
        MissingOperatorInsideGrouping,
        ExpectedRightParenAfterExpression,
        UnmatchedBracketAfterVectorElements,
        ExpectedDictionaryKey,
        ExpectedColonAfterDictionaryKey,
        ExpectedCommaSeparatorInDictionaryLiteral,
        UnmatchedBraceInDictionaryLiteral,
        MissingThenToken,
        MissingElseToken,
        ExpectedLeftParenAfterSwitch,
        ExpectedRightParenAfterSwitchTarget,
        ExpectedLeftBraceBeforeSwitchBody,
        ExpectedRightArrowAfterSwitchCaseIdentifier,
        MissingOperatorInSwitchCaseResult,
        CaseOrDefaultMissingInSwitchAfterResult,
        ExpectedEnumCaseNameAfterCase,
        ExpectedCommaBetweenCaseIdentifiers,
        MultipleDefaultCasesInSwitch,
        ExpectedCaseOrDefaultInsideSwitchBody,
        ExpectedRightBraceAfterSwitchBody,
        MissingOperator,
        TrailingCommaInList,
        MissingCommaOrOperatorBetweenExpressions,
        TrailingComma,
        MissingCommaBetweenFields,
        TopLevelDeclarationInsideFunction,

        // --- Import Resolver Errors ---
        CircularImportDetected,
        ImportFileNotFound
    };

    struct ErrorLocation {
        size_t line;
        size_t column;
        std::string file_path;
    };

    class ValuaScriptException : public std::exception {
    private:
        ErrorCategory category_;
        ErrorCode code_;
        SourceSpan span_;
        std::string message_;

    public:
        ValuaScriptException(ErrorCategory cat, ErrorCode code, SourceSpan span, std::string msg)
            : category_(cat), code_(code), span_(std::move(span)), message_(std::move(msg)) {
        }

        [[nodiscard]] const char *what() const noexcept override {
            return message_.c_str();
        }

        [[nodiscard]] ErrorCategory get_category() const { return category_; }
        [[nodiscard]] ErrorCode get_code() const { return code_; }
        [[nodiscard]] const SourceSpan &get_location() const { return span_; }
    };
}
