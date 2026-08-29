#pragma once
#include "ast_core.h"

namespace valuascript::compiler
{
    class TypeAnnotation : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::TypeAnnotation;
        NodeName name;
        std::vector<TypeAnnPtr> generic_args;

        explicit TypeAnnotation(NodeName n, std::vector<TypeAnnPtr> args = {})
            : AstNode(KIND), name(std::move(n)), generic_args(std::move(args))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return name.is_valid() && span.is_valid() && are_all_valid(generic_args);
        }

    protected:
        TypeAnnotation(AstKind k, NodeName n, std::vector<TypeAnnPtr> args = {})
            : AstNode(k), name(std::move(n)), generic_args(std::move(args))
        {
        }
    };

    class TupleTypeAnnotation : public TypeAnnotation
    {
    public:
        static constexpr AstKind KIND = AstKind::TupleTypeAnnotation;
        std::vector<TypeAnnPtr> element_types;

        explicit TupleTypeAnnotation(std::vector<TypeAnnPtr> elements)
            : TypeAnnotation(KIND, NodeName{"tuple"}), element_types(std::move(elements))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return span.is_valid() && are_all_valid(element_types);
        }
    };
}
