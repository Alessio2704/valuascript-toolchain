#include "type_parser.h"
#include "parser.h"
#include "ast_factory.h"
#include "list_parser.h"
#include "error_recovery.h"

namespace valuascript::compiler
{
    using E = ParserErrorCode;

    TypeParser::TypeParser(Parser& p) : parser(p), ctx(p.ctx), cursor(p.ctx.cursor)
    {
    }

    TypeAnnPtr TypeParser::parse_type_annotation(const ParentBoundaryPredicate& is_at_parent_boundary)
    {
        const Token& start = cursor.peek();

        if (cursor.match({TokenType::LeftParen}))
        {
            CloserTracker tracker(ctx, TokenType::RightParen);

            auto elements = ListParser<TypeAnnPtr>(ctx)
                            .stop_at(TokenType::RightParen)
                            .on_trailing_comma(E::SingleElementTuplesNotAllowed)
                            .on_missing_comma(E::ExpectedCommaSeparatorInTupleType)
                            .is_at_parent_boundary(is_at_parent_boundary)
                            .parse_elements([&]() { return parse_type_annotation(is_at_parent_boundary); });

            Token end_token = cursor.previous();
            try { end_token = cursor.consume(TokenType::RightParen, E::UnmatchedParenthesisInTuple); }
            catch (const ParseSyncException&)
            {
                TokenType peek_type = cursor.peek().type;
                if ((TokenTraits::is_grouping_closer(peek_type) || peek_type == TokenType::Greater) && !ctx.
                    is_active_closer(peek_type))
                    end_token = cursor.advance();
                else end_token = cursor.previous();
            }
            return AstFactory::make_node_with_span<TupleTypeAnnotation>(cursor.make_span(start, end_token),
                                                                        std::move(elements));
        }

        Token name_token = ctx.consume_identifier(E::MissingTypeAnnotation);
        std::vector<TypeAnnPtr> generic_args;

        if (cursor.match({TokenType::Less}))
        {
            generic_args = ListParser<TypeAnnPtr>(ctx)
                           .stop_at(TokenType::Greater)
                           .on_trailing_comma(E::TrailingCommaInGenericArgument)
                           .on_missing_comma(E::ExpectedCommaSeparatorInGenericArgs)
                           .is_at_parent_boundary(is_at_parent_boundary)
                           .parse_elements([&]() { return parse_type_annotation(is_at_parent_boundary); });

            if (generic_args.empty()) cursor.report_error_no_panic(cursor.peek(), E::EmptyGenericTypeAnnotation);
            try { cursor.consume(TokenType::Greater, E::UnmatchedBracketAfterGenericArgs); }
            catch (const ParseSyncException&)
            {
                TokenType peek_type = cursor.peek().type;
                if ((TokenTraits::is_grouping_closer(peek_type) || peek_type == TokenType::Greater) && !ctx.
                    is_active_closer(peek_type))
                    cursor.advance();
            }
        }
        return AstFactory::make_node<TypeAnnotation>(cursor, start, name_token.lexeme, std::move(generic_args));
    }
}
