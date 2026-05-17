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

        ImportPtr parse_import_statement();
        DirectivePtr parse_directive();
        std::vector<Modifier> parse_modifiers(bool is_statement_context = false);
        StructDefPtr parse_struct_definition(std::vector<Modifier> modifiers);
        EnumDefPtr parse_enum_definition(std::vector<Modifier> modifiers);
        TypeAliasPtr parse_type_alias_definition(std::vector<Modifier> modifiers);
        FuncDefPtr parse_function_definition(std::vector<Modifier> modifiers);
        GenericParameter parse_generic_parameter(const ParameterRuleSpec& spec,
                                                 const ParentBoundaryPredicate& is_at_parent_boundary = nullptr);
    };
}
