#include "stages/frontend/parser/parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler {
    void Parser::synchronize_to_closer(TokenType closing_token) {
        int internal_depth = 0;
        while (!cursor_.is_at_end()) {
            const Token &tok = cursor_.peek();
            const TokenType next = cursor_.peek(1).type;

            if (internal_depth == 0) {
                if (tok.type == closing_token) return;

                if (is_grouping_closer(tok.type)) {
                    if (next == TokenType::Comma) {
                        cursor_.advance();
                        return;
                    }
                    return;
                }

                if (is_statement_start(tok, next)) {
                    return;
                }
            }

            if (is_grouping_opener(tok.type)) {
                internal_depth++;
            } else if (is_grouping_closer(tok.type)) {
                internal_depth--;
            }

            cursor_.advance();
            if (internal_depth < 0) internal_depth = 0;
        }
    }

    void Parser::synchronize() {
        while (!cursor_.is_at_end()) {
            if (is_statement_start(cursor_.peek(), cursor_.peek(1).type)) {
                return;
            }
            cursor_.advance();
        }
    }
}
