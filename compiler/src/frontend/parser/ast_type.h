#pragma once
#include "ast_core.h"

namespace valuascript::compiler
{
    class TypeAnnotation : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::TypeAnnotation;
        std::string name;
        std::vector<TypeAnnPtr> generic_args;

        explicit TypeAnnotation(std::string_view n, std::vector<TypeAnnPtr> args = {})
            : AstNode(KIND), name(n), generic_args(std::move(args))
        {
        }

    protected:
        TypeAnnotation(AstKind k, std::string_view n, std::vector<TypeAnnPtr> args = {})
            : AstNode(k), name(n), generic_args(std::move(args))
        {
        }
    };

    class TupleTypeAnnotation : public TypeAnnotation
    {
    public:
        static constexpr AstKind KIND = AstKind::TupleTypeAnnotation;
        std::vector<TypeAnnPtr> element_types;

        explicit TupleTypeAnnotation(std::vector<TypeAnnPtr> elements)
            : TypeAnnotation(KIND, "tuple"), element_types(std::move(elements))
        {
        }
    };
}
