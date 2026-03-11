#pragma once
#include <string>
#include <utility>


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
        ErrorLocation location_;
        std::string message_;
        std::string formatted_message_;

    public:
        ValuaScriptException(ErrorCategory cat, ErrorCode code, ErrorLocation loc, std::string msg)
            : category_(cat), code_(code), location_(std::move(loc)), message_(std::move(msg)) {
            formatted_message_ = "[" + location_.file_path + ":" + std::to_string(location_.line) + ":" + std::to_string(location_.column) + "] " + message_;
        }

        [[nodiscard]] const char *what() const noexcept override { return formatted_message_.c_str(); }
        [[nodiscard]] ErrorCode get_code() const { return code_; }
        [[nodiscard]] ErrorCategory get_category() const { return category_; }
        [[nodiscard]] ErrorLocation get_location() const { return location_; }
    };
}
