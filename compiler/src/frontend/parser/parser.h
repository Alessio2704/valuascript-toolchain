#pragma once

#include <memory>
#include <vector>
#include "parser_context.h"

namespace valuascript::compiler
{
    class ExpressionParser;
    class StatementParser;
    class DeclarationParser;
    class TypeParser;

    enum class ParseContextType { TopLevel, FunctionBody };

    class Parser
    {
    public:
        ParserContext ctx;

        std::unique_ptr<ExpressionParser> expr_parser;
        std::unique_ptr<StatementParser> stmt_parser;
        std::unique_ptr<DeclarationParser> decl_parser;
        std::unique_ptr<TypeParser> type_parser;

        explicit Parser(TokenCursor cursor);
        ~Parser();

        std::unique_ptr<Program> parse_program();

        void parse_statement_or_declaration(ParseContextType parse_ctx, Program* program,
                                            std::vector<std::unique_ptr<Statement>>& block);
        void consume_unexpected_statement_gracefully();
    };
}
