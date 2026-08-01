#include "expression_parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler
{
    bool TokenTraits::is_valid_lvalue(const Expression* target_expression)
    {
        return ast_cast<const IdentifierAccess>(target_expression) ||
            ast_cast<const DotAccess>(target_expression) ||
            ast_cast<const BracketAccess>(target_expression);
    }

    bool TokenTraits::is_expression_start(const TokenType type)
    {
        return ExpressionParser::get_rule(type).prefix != nullptr;
    }

    bool TokenTraits::is_statement_start(const Token& token, TokenType lookahead_type)
    {
        if (acts_like_identifier(token, lookahead_type)) return false;
        return is_top_level_token(token.type);
    }

    bool TokenTraits::is_expression_statement_start(const Token& token, TokenType lookahead_type)
    {
        if (token.type == TokenType::Identifier || token.type == TokenType::Self || acts_like_identifier(token, lookahead_type))
        {
            return lookahead_type == TokenType::Assign ||
                lookahead_type == TokenType::LeftParen ||
                lookahead_type == TokenType::LeftBracket ||
                lookahead_type == TokenType::Dot;
        }
        return false;
    }

    bool TokenTraits::is_newline_statement_boundary(const Token& prev, const Token& current, TokenType next, bool is_greater_container_closer)
    {
        if (current.line <= prev.line) return false;
        if (is_statement_start(current, next)) return true;
        if (current.type == TokenType::Return) return true;
        if (is_expression_statement_start(current, next))
        {
            if (is_greater_container_closer && (prev.type == TokenType::Greater || prev.type == TokenType::Less)) return true;
            if (is_dangling_operator(prev.type) || is_grouping_opener(prev.type)) return false;
            return true;
        }
        return false;
    }

    bool TokenTraits::is_identifier_start(const Token& token)
    {
        return token.type == TokenType::Identifier || (is_reserved_keyword(token) && !
            is_top_level_only_declaration(token.type));
    }

    bool TokenTraits::acts_like_identifier(const Token& token, TokenType lookahead_type)
    {
        if (!is_reserved_keyword(token)) return false;
        return (lookahead_type == TokenType::Comma || lookahead_type == TokenType::Colon ||
            lookahead_type == TokenType::Assign || lookahead_type == TokenType::LeftParen ||
            lookahead_type == TokenType::RightParen || lookahead_type == TokenType::LeftBrace ||
            lookahead_type == TokenType::RightBrace || lookahead_type == TokenType::Less ||
            lookahead_type == TokenType::Greater || lookahead_type == TokenType::Dot ||
            lookahead_type == TokenType::LeftBracket || lookahead_type == TokenType::RightBracket ||
            lookahead_type == TokenType::Arrow || lookahead_type == TokenType::EndOfFile);
    }

    bool TokenTraits::is_top_level_token(const TokenType type)
    {
        switch (type)
        {
        case TokenType::Let:
        case TokenType::Func:
        case TokenType::Struct:
        case TokenType::Enum:
        case TokenType::Typealias:
        case TokenType::Extension:
        case TokenType::Import:
        case TokenType::At:
        case TokenType::Hash: return true;
        default: return false;
        }
    }

    bool TokenTraits::is_grouping_opener(const TokenType type)
    {
        return type == TokenType::LeftParen || type == TokenType::LeftBracket || type == TokenType::LeftBrace;
    }

    bool TokenTraits::is_grouping_closer(const TokenType type)
    {
        return type == TokenType::RightParen || type == TokenType::RightBracket || type == TokenType::RightBrace;
    }

    bool TokenTraits::is_top_level_only_declaration(TokenType type)
    {
        switch (type)
        {
        case TokenType::Import:
        case TokenType::Hash:
        case TokenType::Func:
        case TokenType::Struct:
        case TokenType::Typealias:
        case TokenType::Enum:
        case TokenType::Extension: return true;
        default: return false;
        }
    }

    bool TokenTraits::is_missing_closing_delimiter_error(const ParserErrorCode code)
    {
        switch (code)
        {
        case ParserErrorCode::UnmatchedBracketAfterGenericArgs:
        case ParserErrorCode::UnmatchedParenthesisInTuple:
        case ParserErrorCode::UnmatchedBracketAfterTensorIndex:
        case ParserErrorCode::UnmatchedBracketAfterTensorElements:
        case ParserErrorCode::UnmatchedBraceInDictionaryLiteral:
        case ParserErrorCode::UnmatchedParenthesisAfterModifierArgs:
        case ParserErrorCode::ExpectedRightParenAfterParameters:
        case ParserErrorCode::ExpectedRightParenAfterArguments:
        case ParserErrorCode::ExpectedRightParenAfterTupleElements:
        case ParserErrorCode::ExpectedRightParenAfterExpression:
        case ParserErrorCode::ExpectedRightParenAfterSwitchTarget:
        case ParserErrorCode::ExpectedRightBraceAfterStructBody:
        case ParserErrorCode::ExpectedRightBraceAfterEnumBody:
        case ParserErrorCode::ExpectedRightBraceAfterFunctionBody:
        case ParserErrorCode::ExpectedRightBraceAfterSwitchBody:
        case ParserErrorCode::ExpectedRightBraceAfterExtensionBody:
            return true;
        default:
            return false;
        }
    }
}
