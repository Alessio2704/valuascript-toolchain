#include "stages/frontend/parser/parser.h"

namespace valuascript::compiler {
    Parser::Parser(TokenCursor cursor) : cursor_(std::move(cursor)) {
    }

    std::unique_ptr<Program> Parser::parse_program() {
        auto program = std::make_unique<Program>();
        const Token &start_token = cursor_.peek();
        std::vector<std::unique_ptr<Statement> > dummy_block;

        while (!cursor_.is_at_end()) {
            try {
                parse_statement_or_declaration(ParseContext::TopLevel, program.get(), dummy_block);
            } catch (const ParseSyncException &) {
                synchronize();
            }
        }

        program->span = cursor_.make_span(start_token, cursor_.previous());
        return program;
    }
}