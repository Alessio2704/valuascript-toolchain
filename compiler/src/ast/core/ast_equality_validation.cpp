#include "ast/core/ast_equality.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename T>
    concept EqualityComparableNode = requires(const T& a, const T& b, const T* pa, const T* pb)
    {
        { ast_equals(a, b) } -> std::same_as<bool>;
        { ast_equals(pa, pb) } -> std::same_as<bool>;
    };

    template <EqualityComparableNode T>
    consteval bool verify_equality_comparable_node()
    {
        return true;
    }

    template <typename Tuple>
    struct AstEqualityCompletenessValidator;

    template <typename... Types>
    struct AstEqualityCompletenessValidator<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (verify_equality_comparable_node<Types>() && ...);
        }
    };

    static_assert(AstEqualityCompletenessValidator<AllAstNodeTypes>::validate(),
                  "All registered AST node types in AllAstNodeTypes must support ast_equals()");
}
