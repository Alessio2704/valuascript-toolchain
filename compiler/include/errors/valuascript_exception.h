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

        // --- Parser Errors ---
        MissingValueAfterEquals,
        IncompleteAssignment,
        ExpectedModifierName,
        UnmatchedParenthesis,
        UnmatchedParenthesisInTuple,
        UnmatchedBracket,
        ReservedKeywordAsIdentifier,
        ExpectedLetOrVarToken,
        ExpectedReturnStatement,
        MissingColonAfterParameter,
        MissingColonAfterArgument,
        MissingThenToken,
        MissingElseToken,
        UnexpectedToken,
        ExpectedImportToken,
        MissingImportPathString,
        GeneralParsingError,
        MissingDirectiveName,
        MissingFunctionName,
        MissingParameterName,
        MissingArgumentName,
        MissingTypeAnnotation,
        MissingArrowInFunction,
        ChainingNotAllowedForComparisonOperations,
        EmptyBracketAccess,
        ExpectedDictionaryKey,
        ExpectedColonAfterDictionaryKey,
        ExpectedStructToken,
        ExpectedStructName,
        ExpectedBraceInStructDefinition,
        ExpectedStructFieldName,
        ExpectedColonAfterStructFieldName,
        UnmatchedBraceInDictionaryLiteral,
        ExpectedPropertyName,
        UnterminatedDecimal,
        DecimalMissingLeadingZero,
        ExpectedEnumToken,
        ExpectedEnumName,
        ExpectedColonAfterEnumName,
        ExpectedEnumCaseName,
        ExpectedLeftBrace,
        ExpectedRightBrace,
        ExpectedSwitchKeyword,
        ExpectedLeftParen,
        ExpectedRightParen,
        ExpectedRightArrow,
        ExpectedRightArrowAfterSwitchCaseIdentifier,
        MultipleDefaultCasesInSwitch,
        CaseOrDefaultMissingInSwitch,
        InvalidLeftSideExpressionInReassignment,
        InvalidStandaloneStatement,
        MultiReassignmentNotSupported,
        InvalidExpression,

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
        std::string formatted_message_;

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
