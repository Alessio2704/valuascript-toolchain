#include "ast/equality/ast_disjoint.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler
{
    template <DisjointableAstNode T>
    consteval bool verify_disjointable_ast_node()
    {
        return true;
    }

    template <typename Tuple>
    struct AstDisjointCompletenessValidator;

    template <typename... Types>
    struct AstDisjointCompletenessValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_disjointable_ast_node<Types>() && ...);
        }
    };

    static_assert(AstDisjointCompletenessValidator<AllAstNodeTypes>::validate(),
                  "Every registered AST node in AllAstNodeTypes must be disjointable via ast_is_disjoint()");
}
