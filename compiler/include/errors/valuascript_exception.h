#pragma once
#include <string>
#include <utility>


namespace valuascript::compiler {
    enum class ErrorCategory { Lexical, Syntax, Semantic, Internal };

    enum class ErrorCode {
        // --- Lexer Errors ---
        InvalidCharacter,
        UnclosedString,
        InvalidIdentifier,

        // --- Parser Errors ---
        MissingValueAfterEquals,
        IncompleteAssignment,
        UnmatchedParenthesis,
        UnmatchedParenthesisInTuple,
        UnmatchedBracket,
        ReservedKeywordAsIdentifier,
        ExpectedLetToken,
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
        EmptyVectorAccess,
        ExpectedDictionaryKey,
        ExpectedColonAfterDictionaryKey,
        ExpectedStructToken,
        ExpectedStructName,
        ExpectedBraceInStructDefinition,
        ExpectedStructFieldName,
        ExpectedColonAfterStructFieldName,
        UnmatchedBraceInDictionaryLiteral,
        InvalidExpression
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
            formatted_message_ = "[" + location_.file_path + ":" + std::to_string(location_.line) + "] " + message_;
        }

        [[nodiscard]] const char *what() const noexcept override { return formatted_message_.c_str(); }
        [[nodiscard]] ErrorCode get_code() const { return code_; }
        [[nodiscard]] ErrorCategory get_category() const { return category_; }
        [[nodiscard]] ErrorLocation get_location() const { return location_; }
    };
}
