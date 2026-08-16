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

        if (cursor.check(TokenType::At))
        {
            auto mods = parser.parse_modifiers();
            if (!mods.empty())
            {
                SourceSpan span = cursor.combine_spans(mods.front().span, mods.back().span);
                cursor.report_error_no_panic(span, E::ModifiersAttachedToInvalidDeclaration);
            }
        }

        if (cursor.match(TokenType::LeftParen))
        {
            CloserTracker tracker(ctx, TokenType::RightParen, ContainerKind::TypeTuple);

            auto elements = ListParser<TypeAnnPtr>(ctx)
                            .stop_at(TokenType::RightParen)
                            .on_trailing_comma(E::TrailingCommaInTuple)
                            .on_missing_comma(E::ExpectedCommaSeparatorInTupleType)
                            .is_at_parent_boundary(is_at_parent_boundary)
                            .parse_elements([&]() { return parse_type_annotation(is_at_parent_boundary); });

            Token end_token = cursor.previous();
            if (ErrorRecovery::should_yield_closer_to_parent(ctx, TokenType::RightParen) ||
                (!cursor.check(TokenType::RightParen) &&
                 (ctx.is_active_closer(cursor.peek().type) || (is_at_parent_boundary && is_at_parent_boundary(0)))))
            {
                cursor.report_error_no_panic(cursor.previous(), E::UnmatchedParenthesisInTuple);
                end_token = cursor.previous();
            }
            else if (cursor.check(TokenType::RightParen))
            {
                end_token = cursor.advance();
            }
            else
            {
                cursor.report_error_no_panic(cursor.previous(), E::UnmatchedParenthesisInTuple);
                TokenType peek_type = cursor.peek().type;
                if ((TokenTraits::is_grouping_closer(peek_type) || peek_type == TokenType::Greater) && !ctx.is_active_closer(peek_type))
                    end_token = cursor.advance();
                else end_token = cursor.previous();
            }
            return AstFactory::make_node_with_span<TupleTypeAnnotation>(cursor.make_span(start, end_token),
                                                                        std::move(elements));
        }

        std::vector<TypeAnnPtr> generic_args;
        Token name_token;
        bool missing_base_type = false;

        if (cursor.check(TokenType::Identifier))
        {
            name_token = cursor.advance();
        }
        else if (cursor.check(TokenType::Less))
        {
            missing_base_type = true;
            cursor.report_error_no_panic(cursor.peek(), E::MissingTypeAnnotation);
            name_token = Token(TokenType::Identifier, "", start.line, start.column);
        }
        else
        {
            name_token = ctx.consume_identifier(E::MissingTypeAnnotation, false);
        }

        if (cursor.match(TokenType::Less))
        {
            CloserTracker tracker(ctx, TokenType::Greater, ContainerKind::TypeGeneric);

            generic_args = ListParser<TypeAnnPtr>(ctx)
                           .stop_at(TokenType::Greater)
                           .on_trailing_comma(E::TrailingCommaInGenericArgument)
                           .on_missing_comma(E::ExpectedCommaSeparatorInGenericArgs)
                           .is_at_parent_boundary(is_at_parent_boundary)
                           .parse_elements([&]() { return parse_type_annotation(is_at_parent_boundary); });

            if (generic_args.empty()) cursor.report_error_no_panic(cursor.peek(), E::EmptyGenericTypeAnnotation);

            if (ErrorRecovery::should_yield_closer_to_parent(ctx, TokenType::Greater) ||
                (!cursor.check(TokenType::Greater) &&
                 (ctx.is_active_closer(cursor.peek().type) || (is_at_parent_boundary && is_at_parent_boundary(0)))))
            {
                cursor.report_error_no_panic(cursor.previous(), E::UnmatchedBracketAfterGenericArgs);
            }
            else if (cursor.check(TokenType::Greater))
            {
                cursor.advance();
            }
            else
            {
                cursor.report_error_no_panic(cursor.previous(), E::UnmatchedBracketAfterGenericArgs);
                TokenType peek_type = cursor.peek().type;
                if ((TokenTraits::is_grouping_closer(peek_type) || peek_type == TokenType::Greater) && !ctx.is_active_closer(peek_type))
                    cursor.advance();
            }
        }
        else if (name_token.type != TokenType::Identifier && !is_reserved_keyword(name_token))
        {
            if (!cursor.is_at_end() && &cursor.peek() == &start)
            {
                cursor.advance();
            }
            return nullptr;
        }

        if (missing_base_type) return nullptr;

        return AstFactory::make_node<TypeAnnotation>(cursor, start, name_token.lexeme, std::move(generic_args));
    }
}
