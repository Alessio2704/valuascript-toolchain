#include "parser.h"
#include "expression_parser.h"
#include "statement_parser.h"
#include "declaration_parser.h"
#include "type_parser.h"
#include "error_recovery.h"

namespace valuascript::compiler
{
    using E = ParserErrorCode;

    Parser::Parser(TokenCursor cursor) : ctx(std::move(cursor))
    {
        expr_parser = std::make_unique<ExpressionParser>(*this);
        stmt_parser = std::make_unique<StatementParser>(*this);
        decl_parser = std::make_unique<DeclarationParser>(*this);
        type_parser = std::make_unique<TypeParser>(*this);
        ctx.on_unexpected_statement = [this]() { this->consume_unexpected_statement_gracefully(); };
    }

    Parser::~Parser() = default;

    std::unique_ptr<Program> Parser::parse_program()
    {
        auto program = std::make_unique<Program>();
        program->arena = std::make_unique<AstArena>(256 * 1024);
        std::pmr::memory_resource* prev_resource = AstArenaManager::get_current_resource();
        AstArenaManager::set_current_resource(program->arena->resource());

        const Token& start_token = ctx.cursor.peek();
        std::vector<StmtPtr> dummy_block;

        while (!ctx.cursor.is_at_end())
        {
            ErrorRecovery::attempt_parse_void(
                ctx,
                [&]
                {
                    parse_statement_or_declaration(ParseContextType::TopLevel, program.get(), nullptr, dummy_block);
                },
                RecoveryConfig::ForceStopAtBoundary()
            );
        }

        program->span = ctx.cursor.make_span(start_token, ctx.cursor.previous());
        const auto& raw_comments = ctx.cursor.get_comments();
        program->comments.reserve(raw_comments.size());
        for (const auto& c : raw_comments)
        {
            program->comments.emplace_back(c);
        }
        AstArenaManager::set_current_resource(prev_resource);
        return program;
    }

    void Parser::parse_statement_or_declaration(ParseContextType parse_ctx, Program* program, ExtensionDefinition* extension,
                                                std::vector<StmtPtr>& block)
    {
        const Token& start_token = ctx.cursor.peek();
        std::vector<Modifier> modifiers = decl_parser->parse_modifiers(true);
        TokenType token_type = ctx.cursor.peek().type;
        Program dummy_program;

        if (token_type == TokenType::EndOfFile || (token_type == TokenType::RightBrace && parse_ctx ==
            ParseContextType::FunctionBody))
        {
            ctx.reject_modifiers(modifiers);
            return;
        }

        bool is_disallowed_placement = false;
        std::string disallowed_construct;
        std::string disallowed_context;
        bool prev_suppress = ctx.cursor.get_suppress_errors();

        auto get_construct_str = [](TokenType t) -> std::string {
            switch (t)
            {
            case TokenType::Import: return "'import' statement";
            case TokenType::Hash: return "directive";
            case TokenType::Extension: return "'extension' declaration";
            case TokenType::Func: return "'func' declaration";
            case TokenType::Struct: return "'struct' declaration";
            case TokenType::Enum: return "'enum' declaration";
            case TokenType::Typealias: return "'typealias' declaration";
            case TokenType::Return: return "'return' statement";
            default: return "construct";
            }
        };

        auto get_context_str = [](ParseContextType p_ctx) -> std::string {
            switch (p_ctx)
            {
            case ParseContextType::FunctionBody: return "inside a function body";
            case ParseContextType::ExtensionBody: return "inside an extension body";
            case ParseContextType::TopLevel: return "at top level";
            default: return "in this context";
            }
        };

        if (parse_ctx == ParseContextType::FunctionBody && TokenTraits::is_top_level_only_declaration(token_type))
        {
            is_disallowed_placement = true;
            disallowed_construct = get_construct_str(token_type);
            disallowed_context = get_context_str(parse_ctx);
            parse_ctx = ParseContextType::TopLevel;
            program = &dummy_program;
            ctx.cursor.set_suppress_errors(true);
        }
        else if (parse_ctx == ParseContextType::ExtensionBody)
        {
            if (token_type == TokenType::Import || token_type == TokenType::Hash ||
                token_type == TokenType::Extension || token_type == TokenType::Return)
            {
                is_disallowed_placement = true;
                disallowed_construct = get_construct_str(token_type);
                disallowed_context = get_context_str(parse_ctx);
                ctx.cursor.set_suppress_errors(true);
            }
        }
        else if (parse_ctx == ParseContextType::TopLevel && token_type == TokenType::Return)
        {
            is_disallowed_placement = true;
            disallowed_construct = get_construct_str(token_type);
            disallowed_context = get_context_str(parse_ctx);
            ctx.cursor.set_suppress_errors(true);
        }

        try
        {
            switch (token_type)
            {
            case TokenType::Import:
                if (program)
                    program->import_statements.push_back(decl_parser->parse_import_statement(std::move(modifiers)));
                else
                    decl_parser->parse_import_statement(std::move(modifiers));
                break;
            case TokenType::Hash:
                ctx.reject_modifiers(modifiers);
                if (program)
                    program->directives.push_back(decl_parser->parse_directive());
                else
                    decl_parser->parse_directive();
                break;
            case TokenType::Extension:
                if (program)
                    program->extension_definitions.push_back(
                        decl_parser->parse_extension_definition(std::move(modifiers)));
                else
                    decl_parser->parse_extension_definition(std::move(modifiers));
                break;
            case TokenType::Func:
                if (program)
                    program->function_definitions.push_back(
                        decl_parser->parse_function_definition(std::move(modifiers)));
                else if (extension)
                    extension->function_definitions.push_back(
                        decl_parser->parse_function_definition(std::move(modifiers)));
                break;
            case TokenType::Struct:
                if (program)
                    program->struct_definitions.push_back(
                        decl_parser->parse_struct_definition(std::move(modifiers)));
                else if (extension)
                    extension->struct_definitions.push_back(
                        decl_parser->parse_struct_definition(std::move(modifiers)));
                break;
            case TokenType::Enum:
                if (program)
                    program->enum_definitions.push_back(
                        decl_parser->parse_enum_definition(std::move(modifiers)));
                else if (extension)
                    extension->enum_definitions.push_back(
                        decl_parser->parse_enum_definition(std::move(modifiers)));
                break;
            case TokenType::Typealias:
                if (program)
                    program->type_aliases.push_back(
                        decl_parser->parse_type_alias_definition(std::move(modifiers)));
                else if (extension)
                    extension->type_aliases.push_back(
                        decl_parser->parse_type_alias_definition(std::move(modifiers)));
                break;
            case TokenType::Let:
                {
                    auto assign = stmt_parser->parse_assignment(std::move(modifiers));
                    if (program) program->execution_steps.push_back(std::move(assign));
                    else if (extension) extension->execution_steps.push_back(std::move(assign));
                    else block.push_back(std::move(assign));
                    break;
                }
            case TokenType::Return:
                {
                    auto ret = stmt_parser->parse_return_statement(std::move(modifiers));
                    if (program) program->execution_steps.push_back(std::move(ret));
                    else if (extension) extension->execution_steps.push_back(std::move(ret));
                    else block.push_back(std::move(ret));
                    break;
                }
            default:
                ctx.reject_modifiers(modifiers);
                if (auto expr_stmt = stmt_parser->parse_expression_statement())
                {
                    if (program) program->execution_steps.push_back(std::move(expr_stmt));
                    else if (extension) extension->execution_steps.push_back(std::move(expr_stmt));
                    else block.push_back(std::move(expr_stmt));
                }
                break;
            }
        }
        catch (const ParseSyncException&)
        {
            if (is_disallowed_placement)
            {
                ctx.cursor.set_suppress_errors(prev_suppress);
                ctx.cursor.report_error_no_panic(ctx.cursor.make_span(start_token, ctx.cursor.previous()),
                                                 E::InvalidConstructPlacement, disallowed_construct, disallowed_context);
            }
            throw;
        }

        if (is_disallowed_placement)
        {
            ctx.cursor.set_suppress_errors(prev_suppress);
            ctx.cursor.report_error_no_panic(ctx.cursor.make_span(start_token, ctx.cursor.previous()),
                                             E::InvalidConstructPlacement, disallowed_construct, disallowed_context);
        }
    }

    void Parser::consume_unexpected_statement_gracefully()
    {
        bool prev_suppress = ctx.cursor.get_suppress_errors();
        bool prev_consuming = ctx.is_consuming_unexpected;
        ctx.cursor.set_suppress_errors(true);
        ctx.is_consuming_unexpected = true;
        Program dummy;
        try
        {
            std::vector<StmtPtr> dummy_block;
            parse_statement_or_declaration(ParseContextType::TopLevel, &dummy, nullptr, dummy_block);
        }
        catch (const ParseSyncException&)
        {
        }
        ctx.cursor.set_suppress_errors(prev_suppress);
        ctx.is_consuming_unexpected = prev_consuming;
    }

    ExprPtr Parser::parse_expression(Precedence min_precedence)
    {
        return expr_parser->parse_expression(min_precedence);
    }

    TypeAnnPtr Parser::parse_type_annotation(const std::function<bool(int)>& is_at_parent_boundary)
    {
        return type_parser->parse_type_annotation(is_at_parent_boundary);
    }

    std::vector<Modifier> Parser::parse_modifiers(bool is_statement_context)
    {
        return decl_parser->parse_modifiers(is_statement_context);
    }

    GenericParameter Parser::parse_generic_parameter(const ParameterRuleSpec& spec,
                                                     const std::function<bool(int)>& is_at_parent_boundary)
    {
        return decl_parser->parse_generic_parameter(spec, is_at_parent_boundary);
    }

    void Parser::verify_statement_end() const { stmt_parser->verify_statement_end(); }
}
