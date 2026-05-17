#pragma once
#include "parser_context.h"

namespace valuascript::compiler
{
    class Parser;

    class StatementParser
    {
    public:
        Parser& parser;
        ParserContext& ctx;
        TokenCursor& cursor;

        explicit StatementParser(Parser& p);

        std::unique_ptr<Assignment> parse_assignment(std::vector<Modifier> modifiers);
        StmtPtr parse_expression_statement();
        std::unique_ptr<ReturnStatement> parse_return_statement();

        void verify_statement_end() const;
    };
}
