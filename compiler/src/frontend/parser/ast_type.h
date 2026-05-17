#pragma once
#include "ast_core.h"

namespace valuascript::compiler
{
    class TypeAnnotation : public AstNode
    {
    public:
        std::string name;
        std::vector<TypeAnnPtr> generic_args;

        explicit TypeAnnotation(std::string n, std::vector<TypeAnnPtr> args = {})
            : name(std::move(n)), generic_args(std::move(args))
        {
        }
    };

    class TupleTypeAnnotation : public TypeAnnotation
    {
    public:
        std::vector<TypeAnnPtr> element_types;

        explicit TupleTypeAnnotation(std::vector<TypeAnnPtr> elements)
            : TypeAnnotation("tuple"), element_types(std::move(elements))
        {
        }
    };
}
