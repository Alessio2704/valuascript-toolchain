#include "ast/core/ast_clone.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler
{
    template <typename T>
    struct CheckCloneableSingleType
    {
        static constexpr bool value = [] {
            if constexpr (requires(const T* n) { clone_node(n); })
            {
                return std::is_same_v<decltype(clone_node(std::declval<const T*>())), std::unique_ptr<T>>;
            }
            else if constexpr (requires(const T& n) { clone_node(n); })
            {
                return std::is_same_v<decltype(clone_node(std::declval<const T&>())), T>;
            }
            else
            {
                return false;
            }
        }();
    };

    template <typename Tuple>
    struct ValidateCloneCompleteness;

    template <typename... Types>
    struct ValidateCloneCompleteness<std::tuple<Types...>>
    {
        static constexpr bool value = (CheckCloneableSingleType<Types>::value && ...);
    };

    static_assert(ValidateCloneCompleteness<AllAstNodeTypes>::value,
                  "All registered AST node types in AllAstNodeTypes must be cloneable via clone_node()");
}
