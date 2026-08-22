#include "ast/core/ast_disjoint.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename T>
    concept DisjointComparableNode = requires(const T& a, const T& b, const T* pa, const T* pb)
    {
        { ast_is_disjoint(a, b) } -> std::same_as<bool>;
        { ast_is_disjoint(pa, pb) } -> std::same_as<bool>;
    };

    template <DisjointComparableNode T>
    consteval bool verify_disjoint_comparable_node()
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
            return (verify_disjoint_comparable_node<Types>() && ...);
        }
    };

    static_assert(AstDisjointCompletenessValidator<AllAstNodeTypes>::validate(),
                  "All registered AST node types in AllAstNodeTypes must support ast_is_disjoint()");
}
