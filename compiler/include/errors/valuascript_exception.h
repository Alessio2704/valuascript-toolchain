#pragma once
#include <string>
#include <utility>

#include "stages/frontend/parser/ast.h"

namespace valuascript::compiler {
    enum class ValuascriptErrorCategory { File, Lexical, Syntax, Semantic, Import, Internal };

    enum class ValuascriptErrorCode {
        // --- File Reader Errors ---
        FileNotFound,

        // --- Lexer Errors ---
        InvalidCharacter,
        UnclosedString,
        InvalidIdentifier,
        DecimalMissingLeadingZero,
        UnterminatedDecimal,
        TrailingSeparatorInNumberLiteral,

        // --- Parser Errors ---
        UnexpectedTopLevelToken,
        ExpectedImportToken,
        MissingImportPathString,
        ExpectedHashToken,
        MissingDirectiveName,
        ModifiersAttachedToInvalidDeclaration,
        ModifiersAttachedToMultiAssignmentSingleElements,
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
        MissingDefaultParameterValue,
        NonDefaultParameterAfterDefault,
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
        TrailingCommaInModifier,
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
        UnmatchedBracketAfterTensorElements,
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
        TrailingCommaInGenericArgument,
        MissingCommaBetweenFields,
        TopLevelDeclarationNotAllowedHere,

        // --- Import Resolver Errors ---
        CircularImportDetected,
        ImportFileNotFound
    };

    class ValuaScriptException : public std::exception {
    private:
        ValuascriptErrorCategory category_;
        ValuascriptErrorCode code_;
        SourceSpan span_;
        std::string message_;

    public:
        ValuaScriptException(ValuascriptErrorCategory cat, ValuascriptErrorCode code, SourceSpan span, std::string msg)
            : category_(cat), code_(code), span_(std::move(span)), message_(std::move(msg)) {
        }

        [[nodiscard]] const char *what() const noexcept override {
            return message_.c_str();
        }

        [[nodiscard]] ValuascriptErrorCategory get_category() const { return category_; }
        [[nodiscard]] ValuascriptErrorCode get_code() const { return code_; }
        [[nodiscard]] const SourceSpan &get_span() const { return span_; }
    };
}