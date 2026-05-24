#include "statement_parser.h"
#include "parser.h"
#include "ast_factory.h"
#include "error_recovery.h"

namespace valuascript::compiler
{
    using E = ParserErrorCode;

    namespace
    {
        ExprPtr clone_expression(const Expression* expr);

        Modifier clone_modifier(const Modifier& mod)
        {
            Modifier copy;
            copy.name = mod.name;
            copy.span = mod.span;
            for (const auto& [arg_name, expr] : mod.arguments)
            {
                copy.arguments.push_back({arg_name, clone_expression(expr.get())});
            }
            return copy;
        }

        ExprPtr clone_expression(const Expression* expr)
        {
            if (!expr) return nullptr;

            if (auto* e = dynamic_cast<const NumberLiteral*>(expr))
                return AstFactory::make_node_with_span<NumberLiteral>(e->span, e->value);
            if (auto* e = dynamic_cast<const PercentageLiteral*>(expr))
                return AstFactory::make_node_with_span<PercentageLiteral>(e->span, e->value);
            if (auto* e = dynamic_cast<const StringLiteral*>(expr))
                return AstFactory::make_node_with_span<StringLiteral>(e->span, e->value);
            if (auto* e = dynamic_cast<const BooleanLiteral*>(expr))
                return AstFactory::make_node_with_span<BooleanLiteral>(e->span, e->value);
            if (auto* e = dynamic_cast<const IdentifierAccess*>(expr))
                return AstFactory::make_node_with_span<IdentifierAccess>(e->span, e->name);
            if (auto* e = dynamic_cast<const SelfExpression*>(expr))
                return AstFactory::make_node_with_span<SelfExpression>(e->span);
            if (auto* e = dynamic_cast<const UnaryExpression*>(expr))
                return AstFactory::make_node_with_span<UnaryExpression>(
                    e->span, e->op, clone_expression(e->right.get()));
            if (auto* e = dynamic_cast<const BinaryExpression*>(expr))
                return AstFactory::make_node_with_span<BinaryExpression>(
                    e->span, clone_expression(e->left.get()), e->op, clone_expression(e->right.get()));
            if (auto* e = dynamic_cast<const GroupingExpression*>(expr))
                return AstFactory::make_node_with_span<GroupingExpression>(
                    e->span, clone_expression(e->expression.get()));
            if (auto* e = dynamic_cast<const ConditionalExpression*>(expr))
                return AstFactory::make_node_with_span<ConditionalExpression>(
                    e->span, clone_expression(e->condition.get()), clone_expression(e->then_branch.get()),
                    clone_expression(e->else_branch.get()));
            if (auto* e = dynamic_cast<const BracketAccess*>(expr))
                return AstFactory::make_node_with_span<BracketAccess>(e->span, clone_expression(e->target.get()),
                                                                      clone_expression(e->index.get()));
            if (auto* e = dynamic_cast<const DotAccess*>(expr))
                return AstFactory::make_node_with_span<DotAccess>(e->span, clone_expression(e->target.get()),
                                                                  e->property_name);
            if (auto* e = dynamic_cast<const TupleLiteral*>(expr))
            {
                std::vector<ExprPtr> elems;
                for (const auto& el : e->elements) elems.push_back(clone_expression(el.get()));
                return AstFactory::make_node_with_span<TupleLiteral>(e->span, std::move(elems));
            }
            if (auto* e = dynamic_cast<const TensorLiteral*>(expr))
            {
                std::vector<ExprPtr> elems;
                for (const auto& el : e->elements) elems.push_back(clone_expression(el.get()));
                return AstFactory::make_node_with_span<TensorLiteral>(e->span, std::move(elems));
            }
            if (auto* e = dynamic_cast<const FunctionCall*>(expr))
            {
                std::vector<std::pair<std::string, ExprPtr>> args;
                for (const auto& [n, v] : e->arguments) args.push_back({n, clone_expression(v.get())});
                return AstFactory::make_node_with_span<FunctionCall>(e->span, clone_expression(e->target.get()),
                                                                     std::move(args));
            }
            if (auto* e = dynamic_cast<const DictLiteral*>(expr))
            {
                std::vector<DictItem> items;
                for (const auto& item : e->elements)
                {
                    std::vector<Modifier> mods;
                    for (const auto& m : item.modifiers) mods.push_back(clone_modifier(m));
                    items.push_back({std::move(mods), item.key, clone_expression(item.value.get())});
                }
                return AstFactory::make_node_with_span<DictLiteral>(e->span, std::move(items));
            }
            if (auto* e = dynamic_cast<const SwitchExpression*>(expr))
            {
                std::vector<SwitchCase> cases;
                for (const auto& sc : e->cases)
                {
                    std::vector<Modifier> c_mods;
                    for (const auto& m : sc.modifiers) c_mods.push_back(clone_modifier(m));
                    cases.push_back({std::move(c_mods), sc.identifiers, clone_expression(sc.result.get())});
                }
                std::vector<Modifier> d_mods;
                for (const auto& m : e->default_modifiers) d_mods.push_back(clone_modifier(m));

                return AstFactory::make_node_with_span<SwitchExpression>(
                    e->span, clone_expression(e->target.get()), std::move(cases), std::move(d_mods),
                    clone_expression(e->default_case.get()));
            }

            return nullptr;
        }
    }


    StatementParser::StatementParser(Parser& p) : parser(p), ctx(p.ctx), cursor(p.ctx.cursor)
    {
    }

    void StatementParser::verify_statement_end() const
    {
        if (!cursor.is_at_end() && cursor.peek().line == cursor.previous().line)
        {
            if (TokenTraits::is_expression_start(cursor.peek().type))
            {
                cursor.report_error(cursor.peek(), E::MissingOperator);
            }
        }
    }

    std::unique_ptr<Assignment> StatementParser::parse_assignment(const std::vector<Modifier>& modifiers)
    {
        const Token& start = cursor.peek();
        cursor.consume(TokenType::Let, E::ExpectedLetToken);

        std::vector<AssignmentTarget> targets;
        do
        {
            std::vector<Modifier> target_mods;
            target_mods.reserve(modifiers.size());
            for (const auto& m : modifiers)
            {
                target_mods.push_back(clone_modifier(m));
            }

            auto inner_mods = parser.parse_modifiers();

            target_mods.insert(target_mods.end(),
                               std::make_move_iterator(inner_mods.begin()),
                               std::make_move_iterator(inner_mods.end()));

            Token target = ErrorRecovery::try_consume_identifier(
                ctx,
                E::ExpectedIdentifier,
                RecoveryConfig::StopAtBoundary({TokenType::Colon, TokenType::Comma, TokenType::Assign})
            );

            TypeAnnPtr type_annotation = nullptr;
            if (cursor.match({TokenType::Colon}))
            {
                type_annotation = ErrorRecovery::try_parse<TypeAnnPtr>(
                    ctx,
                    [&]() { return parser.parse_type_annotation(); },
                    RecoveryConfig::StopAtBoundary({TokenType::Comma, TokenType::Assign})
                );
            }
            targets.push_back({std::move(target_mods), target.lexeme, std::move(type_annotation)});

            if (!cursor.match({TokenType::Comma}))
            {
                if (cursor.peek().type == TokenType::Identifier || cursor.peek().type == TokenType::At)
                {
                    if (cursor.peek().line == cursor.previous().line)
                    {
                        cursor.report_error_no_panic(cursor.peek(), E::ExpectedCommaInMultiAssignment);
                        continue;
                    }
                }
                break;
            }
        }
        while (true);

        ExprPtr value = nullptr;
        auto is_at_boundary = [&]()
        {
            return cursor.is_at_end() || TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type) ||
                (cursor.peek().line > cursor.previous().line && TokenTraits::is_expression_statement_start(
                    cursor.peek(), cursor.peek(1).type)) ||
                ctx.is_active_closer(cursor.peek().type);
        };

        if (cursor.match({TokenType::Assign}))
        {
            if (is_at_boundary()) cursor.report_error_no_panic(cursor.peek(), E::MissingValueAfterEquals, false);
            else value = parser.parse_expression();
        }
        else
        {
            const Token& report_at = (cursor.peek().line > cursor.previous().line && is_at_boundary()) || ctx.
                                     is_active_closer(cursor.peek().type)
                                         ? cursor.previous()
                                         : cursor.peek();
            cursor.report_error_no_panic(report_at, E::IncompleteAssignment);
            if (!is_at_boundary() && TokenTraits::is_expression_start(cursor.peek().type))
            {
                try { value = parser.parse_expression(); }
                catch (const ParseSyncException&)
                {
                }
            }
        }

        if (value) verify_statement_end();

        return AstFactory::make_node<Assignment>(cursor, start, std::move(targets), std::move(value));
    }

    StmtPtr StatementParser::parse_expression_statement()
    {
        auto expr = parser.parse_expression();
        const SourceSpan start_span = expr->span;

        if (cursor.match({TokenType::Comma})) cursor.report_error(cursor.previous(), E::MultiReassignmentNotSupported);

        if (cursor.match({TokenType::Assign}))
        {
            if (!TokenTraits::is_valid_lvalue(expr.get()))
            {
                if (expr && expr->is_complete())
                {
                    cursor.report_error(cursor.previous(), E::InvalidLeftSideExpressionInReassignment);
                }
            }

            ExprPtr value = nullptr;
            bool is_pseudo_stmt = TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type) ||
            (cursor.peek().line > cursor.previous().line && cursor.peek().type == TokenType::Identifier && cursor.
                peek(1).type == TokenType::Assign);

            if (cursor.is_at_end() || is_pseudo_stmt)
            {
                cursor.report_error_no_panic(cursor.peek(), E::MissingValueAfterEquals, false);
            }
            else
            {
                value = parser.parse_expression();
            }

            const SourceSpan end_span = value ? value->span : start_span;
            if (value) verify_statement_end();

            return AstFactory::make_node_with_span<Reassignment>(cursor.combine_spans(start_span, end_span),
                                                                 std::move(expr), std::move(value));
        }

        if (dynamic_cast<FunctionCall*>(expr.get()) == nullptr)
        {
            if (expr && expr->is_complete()) cursor.report_error(cursor.previous(), E::InvalidStandaloneStatement);
            return nullptr;
        }

        verify_statement_end();
        return AstFactory::make_node_with_span<ExpressionStatement>(start_span, std::move(expr));
    }

    std::unique_ptr<ReturnStatement> StatementParser::parse_return_statement(std::vector<Modifier> modifiers)
    {
        const Token& start = cursor.advance();
        std::vector<ExprPtr> return_values;

        do
        {
            return_values.push_back(ErrorRecovery::try_parse<ExprPtr>(
                    ctx, [&]() { return parser.parse_expression(); },
                    RecoveryConfig::ForceStopAtBoundary({TokenType::Comma}))
            );
        }
        while (cursor.match({TokenType::Comma}));

        verify_statement_end();
        return AstFactory::make_node<ReturnStatement>(cursor, start, std::move(modifiers), std::move(return_values));
    }
}
