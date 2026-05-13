#include <utility>
#include "parser.h"

namespace valuascript::compiler
{
    GenericParameter Parser::parse_generic_parameter(const ParameterRuleSpec& spec,
                                                     ParentBoundaryPredicate is_at_parent_boundary)
    {
        GenericParameter result;
        const Token& start_token = cursor_.peek();

        if (spec.allow_modifiers || cursor_.check(TokenType::At))
        {
            auto mods = parse_modifiers();
            if (!mods.empty())
            {
                if (!spec.allow_modifiers)
                {
                    SourceSpan span = cursor_.combine_spans(mods.front().span, mods.back().span);
                    cursor_.report_error_no_panic(span, spec.unexpected_modifier_err);
                }
                else
                {
                    result.modifiers = std::move(mods);
                }
            }
        }

        bool name_failed = false;
        try
        {
            result.name = consume_identifier(spec.missing_name_err, false);
        }
        catch (const ParseSyncException&)
        {
            name_failed = true;
            synchronize_with({
                .stop_tokens = {TokenType::Colon, spec.value_separator, TokenType::Comma},
                .stop_at_statement_boundary_respecting_dangling_op = true
            });
        }

        if (spec.allow_type)
        {
            if (cursor_.check(TokenType::Colon))
            {
                cursor_.advance();
                try
                {
                    result.type = parse_type_annotation(std::move(is_at_parent_boundary));
                }
                catch (const ParseSyncException&)
                {
                    synchronize_with({
                        .stop_tokens = {spec.value_separator, TokenType::Comma},
                        .stop_at_statement_boundary_respecting_dangling_op = true
                    });
                }
            }
            else if (spec.require_type && !name_failed)
            {
                if (!cursor_.check(spec.value_separator) && !cursor_.check(TokenType::Comma) && !is_active_closer(
                    cursor_.peek().type))
                {
                    cursor_.consume(TokenType::Colon, spec.missing_type_colon_err);
                }
                else
                {
                    cursor_.report_error_no_panic(cursor_.peek(), spec.missing_type_colon_err);
                }
            }
        }

        if (spec.allow_value)
        {
            bool has_sep = false;
            if (cursor_.match({spec.value_separator}))
            {
                has_sep = true;
                result.has_value_separator = true;
            }
            else if (spec.require_value && !name_failed)
            {
                if (!cursor_.check(TokenType::Comma) && !is_active_closer(cursor_.peek().type))
                {
                    cursor_.consume(spec.value_separator, spec.missing_value_separator_err);
                }
                else
                {
                    cursor_.report_error_no_panic(cursor_.peek(), spec.missing_value_separator_err);
                }
            }

            if (has_sep || spec.require_value)
            {
                if (cursor_.check(TokenType::Comma) || is_active_closer(cursor_.peek().type))
                {
                    if (has_sep)
                    {
                        cursor_.report_error_no_panic(cursor_.peek(), spec.missing_value_err);
                    }
                }
                else
                {
                    try
                    {
                        result.value = parse_expression();

                        if ((TokenTraits::is_expression_start(cursor_.peek().type) ||
                                TokenTraits::is_binary_operator(cursor_.peek().type)) &&
                            cursor_.peek(1).type != spec.value_separator &&
                            cursor_.peek(1).type != TokenType::Colon)
                        {
                            if (!TokenTraits::is_newline_statement_boundary(
                                cursor_.previous(), cursor_.peek(), cursor_.peek(1).type))
                            {
                                cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperator);
                            }
                        }
                    }
                    catch (const ParseSyncException&)
                    {
                        synchronize_with({
                            .stop_tokens = {TokenType::Comma},
                            .stop_at_statement_boundary_respecting_dangling_op = true
                        });
                    }
                }
            }
        }

        result.span = cursor_.make_span(start_token, cursor_.previous());
        return result;
    }
}
