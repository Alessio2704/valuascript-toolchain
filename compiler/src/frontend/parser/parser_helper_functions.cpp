#include "parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler
{
    const Token& Parser::consume_identifier(ValuascriptErrorCode fallback_err, bool allow_top_level_keywords,
                                            bool check_statement_boundary)
    {
        if (check_statement_boundary && cursor_.peek().line > cursor_.previous().line &&
            TokenTraits::is_expression_statement_start(cursor_.peek(), cursor_.peek(1).type))
        {
            cursor_.report_error(cursor_.peek(), fallback_err);
        }
        if (cursor_.check(TokenType::Identifier))
        {
            return cursor_.advance();
        }

        if (is_reserved_keyword(cursor_.peek()))
        {
            TokenType next = cursor_.peek(1).type;

            bool acts_like_id = TokenTraits::acts_like_identifier(cursor_.peek(), next);
            bool forms_statement = TokenTraits::is_statement_start(cursor_.peek(), next) ||
            (check_statement_boundary && cursor_.peek().line > cursor_.previous().line &&
                TokenTraits::is_expression_statement_start(cursor_.peek(), next));

            if (acts_like_id || !allow_top_level_keywords || !forms_statement)
            {
                cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::ReservedKeywordAsIdentifier, true);
                return cursor_.advance();
            }
        }

        return cursor_.consume(TokenType::Identifier, fallback_err, false);
    }

    void Parser::verify_statement_end() const
    {
        if (!cursor_.is_at_end() && cursor_.peek().line == cursor_.previous().line)
        {
            if (TokenTraits::is_expression_start(cursor_.peek().type))
            {
                cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperator);
            }
        }
    }

    std::vector<std::unique_ptr<Expression>> Parser::parse_expression_list(
        const TokenType closing_token,
        const std::optional<ValuascriptErrorCode> trailing_comma_err,
        const std::vector<TokenType> recovery_boundaries)
    {
        return parse_list<std::unique_ptr<Expression>>(
            closing_token,
            trailing_comma_err,
            ValuascriptErrorCode::MissingCommaOrOperatorBetweenExpressions,
            recovery_boundaries,
            [this]() { return TokenTraits::is_expression_start(cursor_.peek().type); },
            [this]() { return parse_expression(); }
        );
    }

    void Parser::consume_unexpected_statement_gracefully()
    {
        bool prev_suppress = cursor_.get_suppress_errors();
        cursor_.set_suppress_errors(true);

        Program dummy;
        try
        {
            std::vector<std::unique_ptr<Statement>> dummy_block;
            parse_statement_or_declaration(ParseContext::TopLevel, &dummy, dummy_block);
        }
        catch (const ParseSyncException&)
        {
        }

        cursor_.set_suppress_errors(prev_suppress);
    }

    TokenType Parser::peek_past_modifiers() const
    {
        size_t offset = 0;
        while (cursor_.peek(offset).type == TokenType::At)
        {
            offset++;
            if (cursor_.peek(offset).type == TokenType::EndOfFile) break;
            offset++;

            if (cursor_.peek(offset).type == TokenType::LeftParen)
            {
                int depth = 1;
                offset++;
                while (depth > 0 && cursor_.peek(offset).type != TokenType::EndOfFile)
                {
                    if (cursor_.peek(offset).type == TokenType::LeftParen) depth++;
                    else if (cursor_.peek(offset).type == TokenType::RightParen) depth--;
                    offset++;
                }
            }
        }
        return cursor_.peek(offset).type;
    }

    bool Parser::is_at_top_level_declaration() const
    {
        return TokenTraits::is_top_level_only_declaration(peek_past_modifiers());
    }

    bool Parser::is_at_any_declaration() const
    {
        TokenType t = peek_past_modifiers();
        if (t == TokenType::Let) return true;
        return TokenTraits::is_top_level_only_declaration(t);
    }

    bool Parser::is_missing_closing_brace() const
    {
        int depth = 1;
        size_t offset = 0;

        while (true)
        {
            TokenType t = cursor_.peek(offset).type;

            if (t == TokenType::EndOfFile) return true;

            if (t == TokenType::LeftBrace)
            {
                depth++;
            }
            else if (t == TokenType::RightBrace)
            {
                depth--;
                if (depth == 0) return false;
            }
            offset++;
        }
    }

    bool Parser::is_expression_complete(const Expression* expr)
    {
        if (!expr) return false;

        if (auto* b = dynamic_cast<const BinaryExpression*>(expr))
        {
            return is_expression_complete(b->left.get()) &&
                is_expression_complete(b->right.get());
        }

        if (auto* u = dynamic_cast<const UnaryExpression*>(expr))
        {
            return is_expression_complete(u->right.get());
        }

        if (auto* g = dynamic_cast<const GroupingExpression*>(expr))
        {
            return is_expression_complete(g->expression.get());
        }

        if (auto* c = dynamic_cast<const ConditionalExpression*>(expr))
        {
            return is_expression_complete(c->condition.get()) &&
                is_expression_complete(c->then_branch.get()) &&
                is_expression_complete(c->else_branch.get());
        }

        if (auto* f = dynamic_cast<const FunctionCall*>(expr))
        {
            if (!is_expression_complete(f->target.get())) return false;
            for (const auto& [name, val] : f->arguments)
            {
                if (!is_expression_complete(val.get())) return false;
            }
            return true;
        }

        if (auto* d = dynamic_cast<const DictLiteral*>(expr))
        {
            for (const auto& item : d->elements)
            {
                if (!is_expression_complete(item.value.get())) return false;
            }
            return true;
        }

        if (auto* t = dynamic_cast<const TensorLiteral*>(expr))
        {
            for (const auto& elem : t->elements)
            {
                if (!is_expression_complete(elem.get())) return false;
            }
            return true;
        }

        if (auto* tup = dynamic_cast<const TupleLiteral*>(expr))
        {
            for (const auto& elem : tup->elements)
            {
                if (!is_expression_complete(elem.get())) return false;
            }
            return true;
        }

        if (auto* br = dynamic_cast<const BracketAccess*>(expr))
        {
            return is_expression_complete(br->target.get()) &&
                is_expression_complete(br->index.get());
        }

        if (auto* dot = dynamic_cast<const DotAccess*>(expr))
        {
            return is_expression_complete(dot->target.get());
        }

        if (auto* sw = dynamic_cast<const SwitchExpression*>(expr))
        {
            if (!is_expression_complete(sw->target.get())) return false;
            for (const auto& [ids, result] : sw->cases)
            {
                if (!is_expression_complete(result.get())) return false;
            }
            if (sw->default_case && !is_expression_complete(sw->default_case.get())) return false;
            return true;
        }

        if (dynamic_cast<const NumberLiteral*>(expr) ||
            dynamic_cast<const PercentageLiteral*>(expr) ||
            dynamic_cast<const StringLiteral*>(expr) ||
            dynamic_cast<const BooleanLiteral*>(expr) ||
            dynamic_cast<const IdentifierAccess*>(expr) ||
            dynamic_cast<const SelfExpression*>(expr))
        {
            return true;
        }

        return true;
    }
}
