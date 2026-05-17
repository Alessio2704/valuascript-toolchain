#pragma once
#include <memory>
#include <vector>
#include "parser_context.h"
#include "declaration_rules.h"

namespace valuascript::compiler
{
    class ExpressionParser;
    class StatementParser;
    class DeclarationParser;
    class TypeParser;

    enum class ParseContextType { TopLevel, FunctionBody };

    class Parser
    {
    private:
        std::unique_ptr<ExpressionParser> expr_parser;
        std::unique_ptr<StatementParser> stmt_parser;
        std::unique_ptr<DeclarationParser> decl_parser;
        std::unique_ptr<TypeParser> type_parser;

    public:
        ParserContext ctx;

        explicit Parser(TokenCursor cursor);
        ~Parser();

        std::unique_ptr<Program> parse_program();
        void parse_statement_or_declaration(ParseContextType parse_ctx, Program* program,
                                            std::vector<std::unique_ptr<Statement>>& block);
        void consume_unexpected_statement_gracefully();

        std::unique_ptr<Expression> parse_expression(Precedence min_precedence = Precedence::Or);
        std::unique_ptr<TypeAnnotation> parse_type_annotation(
            const std::function<bool(int)>& is_at_parent_boundary = nullptr);
        std::vector<Modifier> parse_modifiers(bool is_statement_context = false);
        GenericParameter parse_generic_parameter(const ParameterRuleSpec& spec,
                                                 const std::function<bool(int)>& is_at_parent_boundary = nullptr);
        void verify_statement_end() const;
    };
}
