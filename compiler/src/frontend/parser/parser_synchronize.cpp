#include "parser.h"
#include <algorithm>

namespace valuascript::compiler {
    bool Parser::is_active_closer(TokenType type) const {
        return std::find(active_closers_.begin(), active_closers_.end(), type) != active_closers_.end();
    }

    void Parser::recover(const SyncPredicate &stop_condition) {
        int depth = 0;
        while (!cursor_.is_at_end()) {
            TokenType t = cursor_.peek().type;

            if (stop_condition(t, depth)) return;

            if (TokenTraits::is_grouping_opener(t)) {
                depth++;
            } else if (TokenTraits::is_grouping_closer(t)) {
                depth--;
            }

            cursor_.advance();
            if (depth < 0) depth = 0;
        }
    }

    void Parser::synchronize_to_closer(TokenType closing_token) {
        recover([&](TokenType t, int depth) {
            if (depth == 0) {
                if (t == closing_token) return true;
                if (TokenTraits::is_grouping_closer(t)) {
                    if (cursor_.peek(1).type == TokenType::Comma) cursor_.advance();
                    return true;
                }
                if (TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type)) return true;
                if (cursor_.peek().line > cursor_.previous().line &&
                    TokenTraits::is_expression_statement_start(cursor_.peek(), cursor_.peek(1).type))
                    return true;
                if (t == TokenType::Then || t == TokenType::Else || t == TokenType::Case || t == TokenType::Default)
                    return true;
            }
            return false;
        });
    }

    void Parser::synchronize_and_consume_closer(TokenType expected_closer) {
        synchronize_to_closer(expected_closer);
        if (cursor_.check(expected_closer)) {
            cursor_.advance();
        } else if (TokenTraits::is_grouping_closer(cursor_.peek().type) && !is_active_closer(cursor_.peek().type)) {
            cursor_.advance();
        }
    }

    void Parser::synchronize_to_switch_boundary() {
        recover([&](TokenType t, int depth) {
            if (depth == 0) {
                if (t == TokenType::Case || t == TokenType::Default || t == TokenType::RightBrace) return true;
                if (is_missing_closing_brace() && (
                        is_at_top_level_declaration() || t == TokenType::Return ||
                        TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
                        (cursor_.peek().line > cursor_.previous().line &&
                         TokenTraits::is_expression_statement_start(cursor_.peek(), cursor_.peek(1).type))))
                    return true;
            }
            return false;
        });
    }

    void Parser::synchronize_to_conditional_boundary() {
        recover([&](TokenType t, int depth) {
            if (depth == 0) {
                if (t == TokenType::Then || t == TokenType::Else || TokenTraits::is_grouping_closer(t)) return true;
                if (is_missing_closing_brace() && (
                        is_at_top_level_declaration() || t == TokenType::Return ||
                        TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
                        (cursor_.peek().line > cursor_.previous().line &&
                         TokenTraits::is_expression_statement_start(cursor_.peek(), cursor_.peek(1).type))))
                    return true;
            }
            return false;
        });
    }

    void Parser::synchronize_block_statement() {
        recover([&](TokenType t, int /*depth*/) {
            return t == TokenType::RightBrace || t == TokenType::Return ||
                   TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
                   (cursor_.peek().line > cursor_.previous().line &&
                    TokenTraits::is_expression_statement_start(cursor_.peek(), cursor_.peek(1).type));
        });
    }

    void Parser::synchronize() {
        while (!cursor_.is_at_end()) {
            if (TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type)) return;
            if (cursor_.peek().line > cursor_.previous().line &&
                TokenTraits::is_expression_statement_start(cursor_.peek(), cursor_.peek(1).type)) {
                return;
            }
            cursor_.advance();
        }
    }
}
