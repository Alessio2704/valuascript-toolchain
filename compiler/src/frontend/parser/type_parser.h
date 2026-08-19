#pragma once
#include "parser_context.h"

namespace valuascript::compiler
{
    class Parser;

    class TypeParser
    {
    public:
        Parser& parser;
        ParserContext& ctx;
        TokenCursor& cursor;

        explicit TypeParser(Parser& p);

        using ParentBoundaryPredicate = std::function<bool(int lookahead)>;

        TypeAnnPtr parse_type_annotation(const ParentBoundaryPredicate& is_at_parent_boundary = nullptr);
    };
}
