#pragma once
#include "parser_context.h"
#include "declaration_rules.h"

namespace valuascript::compiler
{
    class Parser;

    class DeclarationParser
    {
    public:
        Parser& parser;
        ParserContext& ctx;
        TokenCursor& cursor;

        explicit DeclarationParser(Parser& p);

        using ParentBoundaryPredicate = std::function<bool(int lookahead)>;

        std::unique_ptr<ImportStatement> parse_import_statement();
        std::unique_ptr<Directive> parse_directive();
        std::vector<Modifier> parse_modifiers(bool is_statement_context = false);
        std::unique_ptr<StructDefinition> parse_struct_definition(std::vector<Modifier> modifiers);
        std::unique_ptr<EnumDefinition> parse_enum_definition(std::vector<Modifier> modifiers);
        std::unique_ptr<TypeAliasDefinition> parse_type_alias_definition(std::vector<Modifier> modifiers);
        std::unique_ptr<FunctionDefinition> parse_function_definition(std::vector<Modifier> modifiers);
        GenericParameter parse_generic_parameter(const ParameterRuleSpec& spec,
                                                 const ParentBoundaryPredicate& is_at_parent_boundary = nullptr);
    };
}
